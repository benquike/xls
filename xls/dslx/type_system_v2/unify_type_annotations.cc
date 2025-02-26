// Copyright 2025 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "xls/dslx/type_system_v2/unify_type_annotations.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

#include "absl/functional/function_ref.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/substitute.h"
#include "xls/common/status/status_macros.h"
#include "xls/dslx/errors.h"
#include "xls/dslx/frontend/ast.h"
#include "xls/dslx/frontend/pos.h"
#include "xls/dslx/interp_value.h"
#include "xls/dslx/type_system_v2/evaluator.h"
#include "xls/dslx/type_system_v2/inference_table.h"
#include "xls/dslx/type_system_v2/type_annotation_utils.h"

namespace xls::dslx {
namespace {

// A size and signedness with a flag for whether it is automatic. Automatic
// values have more flexible unification rules.
struct SignednessAndSize {
  bool is_auto;
  bool is_signed;
  int64_t size;
};

const TypeAnnotation* SignednessAndSizeToAnnotation(
    Module& module, const SignednessAndSize& signedness_and_size,
    const Span& span) {
  return CreateUnOrSnAnnotation(module, span, signedness_and_size.is_signed,
                                signedness_and_size.size);
}

class Unifier {
 public:
  Unifier(Module& module, InferenceTable& table, const FileTable& file_table,
          UnificationErrorGenerator& error_generator, Evaluator& evaluator,
          ParametricStructInstantiator& parametric_struct_instantiator,
          IndirectAnnotationResolver& indirect_annotation_resolver,
          std::optional<const ParametricContext*> parametric_context,
          std::optional<absl::FunctionRef<bool(const TypeAnnotation*)>>
              accept_predicate)
      : module_(module),
        table_(table),
        file_table_(file_table),
        error_generator_(error_generator),
        evaluator_(evaluator),
        parametric_struct_instantiator_(parametric_struct_instantiator),
        indirect_annotation_resolver_(indirect_annotation_resolver),
        parametric_context_(parametric_context),
        accept_predicate_(accept_predicate) {}

  // Overload that unifies specific type annotations.
  absl::StatusOr<const TypeAnnotation*> UnifyTypeAnnotations(
      std::vector<const TypeAnnotation*> annotations, const Span& span) {
    XLS_RETURN_IF_ERROR(
        indirect_annotation_resolver_.ResolveIndirectTypeAnnotations(
            parametric_context_, annotations, accept_predicate_));
    // Remove all singular `Any` annotations, and if that's all we had, the
    // result is one singular `Any`.
    FilterAnnotations(annotations, [&](const TypeAnnotation* annotation) {
      const auto* any_annotation =
          dynamic_cast<const AnyTypeAnnotation*>(annotation);
      return any_annotation == nullptr || any_annotation->multiple();
    });
    if (annotations.empty()) {
      return module_.Make<AnyTypeAnnotation>();
    }
    // Remove all multiple `Any` annotations, and if that's all we had, the
    // result is one multiple `Any`.
    FilterAnnotations(annotations, [&](const TypeAnnotation* annotation) {
      return dynamic_cast<const AnyTypeAnnotation*>(annotation) == nullptr;
    });
    if (annotations.empty()) {
      return module_.Make<AnyTypeAnnotation>(/*multiple=*/true);
    }
    if (annotations.size() == 1) {
      XLS_ASSIGN_OR_RETURN(std::optional<StructOrProcRef> struct_or_proc_ref,
                           GetStructOrProcRef(annotations[0], file_table_));
      if (!struct_or_proc_ref.has_value()) {
        // This is here mainly for preservation of shorthand annotations
        // appearing in the source code, in case they get put in subsequent
        // error messages. General unification would normalize the format.
        return annotations[0];
      }
    }
    if (const auto* first_tuple_annotation =
            dynamic_cast<const TupleTypeAnnotation*>(annotations[0])) {
      std::vector<const TupleTypeAnnotation*> tuple_annotations;
      tuple_annotations.reserve(annotations.size());
      for (const TypeAnnotation* annotation : annotations) {
        const auto* tuple_annotation =
            dynamic_cast<const TupleTypeAnnotation*>(annotation);
        if (tuple_annotation == nullptr) {
          return error_generator_.TypeMismatchError(parametric_context_,
                                                    annotations[0], annotation);
        }
        tuple_annotations.push_back(tuple_annotation);
      }
      return UnifyTupleTypeAnnotations(tuple_annotations, span);
    }
    if (const auto* first_array_annotation =
            CastToNonBitsArrayTypeAnnotation(annotations[0])) {
      std::vector<const ArrayTypeAnnotation*> array_annotations;
      for (int i = 0; i < annotations.size(); i++) {
        const auto* array_annotation =
            CastToNonBitsArrayTypeAnnotation(annotations[i]);
        if (array_annotation == nullptr) {
          return error_generator_.TypeMismatchError(
              parametric_context_, annotations[0], annotations[i]);
        }
        array_annotations.push_back(array_annotation);
      }
      return UnifyArrayTypeAnnotations(parametric_context_, array_annotations,
                                       span, accept_predicate_);
    }
    if (const auto* first_function_annotation =
            dynamic_cast<const FunctionTypeAnnotation*>(annotations[0])) {
      std::vector<const FunctionTypeAnnotation*> function_annotations;
      function_annotations.reserve(annotations.size());
      for (int i = 0; i < annotations.size(); i++) {
        const auto* function_annotation =
            dynamic_cast<const FunctionTypeAnnotation*>(annotations[i]);
        if (function_annotation == nullptr) {
          return error_generator_.TypeMismatchError(
              parametric_context_, annotations[0], annotations[i]);
        }
        VLOG(5) << "Annotation " << i << ": "
                << function_annotation->ToString();
        function_annotations.push_back(function_annotation);
      }
      return UnifyFunctionTypeAnnotations(function_annotations, span);
    }
    XLS_ASSIGN_OR_RETURN(std::optional<StructOrProcRef> first_struct_or_proc,
                         GetStructOrProcRef(annotations[0], file_table_));
    if (first_struct_or_proc.has_value()) {
      const auto* struct_def =
          dynamic_cast<const StructDef*>(first_struct_or_proc->def);
      CHECK(struct_def != nullptr);
      std::vector<const TypeAnnotation*> annotations_to_unify;
      for (const TypeAnnotation* annotation : annotations) {
        XLS_ASSIGN_OR_RETURN(std::optional<StructOrProcRef> next_struct_or_proc,
                             GetStructOrProcRef(annotation, file_table_));
        if (!next_struct_or_proc.has_value() ||
            next_struct_or_proc->def != struct_def) {
          return error_generator_.TypeMismatchError(parametric_context_,
                                                    annotations[0], annotation);
        }
        if (struct_def->IsParametric()) {
          annotations_to_unify.push_back(annotation);
        }
      }
      // A non-parametric struct is trivially unifiable, because nothing can
      // validly vary between the annotations. A parametric struct needs to have
      // its parameters unified.
      return annotations_to_unify.empty()
                 ? annotations[0]
                 : UnifyParametricStructAnnotations(*struct_def,
                                                    annotations_to_unify);
    }
    return UnifyBitsLikeTypeAnnotations(annotations, span);
  }

 private:
  // Unifies multiple annotations for a tuple. This function assumes the
  // passed-in array is nonempty. It attempts to expand any `AnyTypeAnnotations`
  // with `multiple=true` to match the size of the first annotation.. Unifying a
  // tuple type amounts to unifying the annotations for each member.
  absl::StatusOr<const TupleTypeAnnotation*> UnifyTupleTypeAnnotations(
      std::vector<const TupleTypeAnnotation*> annotations, const Span& span) {
    const int member_count = annotations[0]->members().size();
    std::vector<const TupleTypeAnnotation*> expanded_annotations;

    for (const TupleTypeAnnotation* tuple_annotation : annotations) {
      if (tuple_annotation->members().size() != member_count) {
        // If the sizes don't match, the annotation must contain an
        // `AnyAnnotation` with `multiple = true`. Attempt to expand the
        // multiple any to match the size.
        std::vector<TypeAnnotation*> expanded_members;
        expanded_members.reserve(member_count);
        int delta = member_count - tuple_annotation->members().size();
        for (auto* member : tuple_annotation->members()) {
          const auto* any_annotation = dynamic_cast<AnyTypeAnnotation*>(member);
          if (any_annotation != nullptr && any_annotation->multiple()) {
            for (int i = 0; i < delta + 1; i++) {
              expanded_members.push_back(module_.Make<AnyTypeAnnotation>());
            }
          } else {
            expanded_members.push_back(member);
          }
        }
        tuple_annotation = module_.Make<TupleTypeAnnotation>(
            tuple_annotation->span(), expanded_members);
      }
      CHECK_EQ(tuple_annotation->members().size(), member_count);
      expanded_annotations.push_back(tuple_annotation);
    }

    std::vector<TypeAnnotation*> unified_member_annotations(member_count);
    for (int i = 0; i < member_count; i++) {
      std::vector<const TypeAnnotation*> annotations_for_member;
      annotations_for_member.reserve(annotations.size());
      for (const TupleTypeAnnotation* annotation : expanded_annotations) {
        annotations_for_member.push_back(annotation->members()[i]);
      }
      XLS_ASSIGN_OR_RETURN(const TypeAnnotation* unified_member_annotation,
                           UnifyTypeAnnotations(annotations_for_member, span));
      unified_member_annotations[i] =
          const_cast<TypeAnnotation*>(unified_member_annotation);
    }
    return module_.Make<TupleTypeAnnotation>(span, unified_member_annotations);
  }

  // Unifies multiple annotations for an array. This function assumes the
  // passed-in array is nonempty. Unifying an array type amounts to unifying the
  // element types and dims.
  absl::StatusOr<const ArrayTypeAnnotation*> UnifyArrayTypeAnnotations(
      std::optional<const ParametricContext*> parametric_context_,
      std::vector<const ArrayTypeAnnotation*> annotations, const Span& span,
      std::optional<absl::FunctionRef<bool(const TypeAnnotation*)>>
          accept_predicate) {
    std::vector<const TypeAnnotation*> element_type_annotations;
    std::optional<SignednessAndSize> unified_dim;
    for (int i = 0; i < annotations.size(); i++) {
      const ArrayTypeAnnotation* annotation = annotations[i];
      element_type_annotations.push_back(annotation->element_type());

      XLS_ASSIGN_OR_RETURN(
          int64_t current_dim,
          evaluator_.EvaluateU32OrExpr(parametric_context_, annotation->dim()));
      // This flag indicates we are unifying one min dim with one explicit dim,
      // which warrants a possible different error message than other scenarios.
      const bool is_min_vs_explicit =
          unified_dim.has_value() &&
          (unified_dim->is_auto ^ annotation->dim_is_min());
      absl::StatusOr<SignednessAndSize> new_unified_dim =
          UnifySignednessAndSize(
              parametric_context_, unified_dim,
              SignednessAndSize{.is_auto = annotation->dim_is_min(),
                                .is_signed = false,
                                .size = current_dim},
              annotations[0], annotations[i]);
      if (!new_unified_dim.ok()) {
        // We can only get here when i >= 1, because the 0th annotation can't be
        // a contradiction of preceding info.
        CHECK_GE(i, 1);
        if (is_min_vs_explicit) {
          return TypeInferenceErrorStatus(
              span, /*type=*/nullptr,
              "Annotated array size is too small for explicit element count.",
              file_table_);
        }
        return error_generator_.TypeMismatchError(
            parametric_context_, annotations[i], annotations[i - 1]);
      }
      unified_dim = *new_unified_dim;
    }
    if (unified_dim->is_auto) {
      // This means the only type annotation for the array was fabricated
      // based on an elliptical RHS.
      return TypeInferenceErrorStatus(
          span, /*type=*/nullptr,
          "Array has ellipsis (`...`) but does not have a type annotation.",
          file_table_);
    }
    XLS_ASSIGN_OR_RETURN(const TypeAnnotation* unified_element_type,
                         UnifyTypeAnnotations(element_type_annotations, span));
    XLS_ASSIGN_OR_RETURN(
        Number * size_expr,
        MakeTypeCheckedNumber(
            module_, table_, annotations[0]->span(), unified_dim->size,
            CreateU32Annotation(module_, annotations[0]->span())));
    return module_.Make<ArrayTypeAnnotation>(
        span, const_cast<TypeAnnotation*>(unified_element_type), size_expr);
  }

  // Unifies multiple annotations for a function type. This assumes the
  // passed-in array is nonempty. Unifying a function type amounts to unifying
  // the return type and the argument types.
  absl::StatusOr<const FunctionTypeAnnotation*> UnifyFunctionTypeAnnotations(
      std::vector<const FunctionTypeAnnotation*> annotations,
      const Span& span) {
    VLOG(6) << "UnifyFunctionTypeAnnotations: " << annotations.size();
    // Plausible return types for the function.
    std::vector<const TypeAnnotation*> return_types;
    return_types.reserve(annotations.size());
    // Plausible types for each function argument (rows are argument indices).
    std::vector<std::vector<const TypeAnnotation*>> param_types;
    param_types.resize(annotations[0]->param_types().size());
    for (const FunctionTypeAnnotation* annotation : annotations) {
      VLOG(6) << "Return type to unify: "
              << annotation->return_type()->ToString();
      return_types.push_back(annotation->return_type());
      if (annotation->param_types().size() !=
          annotations[0]->param_types().size()) {
        return ArgCountMismatchErrorStatus(
            span,
            absl::Substitute("Expected $0 argument(s) but got $1.",
                             annotation->param_types().size(),
                             annotations[0]->param_types().size()),
            file_table_);
      }
      for (int i = 0; i < annotation->param_types().size(); i++) {
        const TypeAnnotation* param_type = annotation->param_types()[i];
        VLOG(6) << "Param type " << i
                << " to unify: " << param_type->ToString();
        param_types[i].push_back(param_type);
      }
    }

    // Unify the return type and argument types.
    XLS_ASSIGN_OR_RETURN(const TypeAnnotation* unified_return_type,
                         UnifyTypeAnnotations(return_types, span));
    std::vector<TypeAnnotation*> unified_param_types;
    unified_param_types.reserve(param_types.size());
    for (const std::vector<const TypeAnnotation*>& argument : param_types) {
      XLS_ASSIGN_OR_RETURN(const TypeAnnotation* unified_param_type,
                           UnifyTypeAnnotations(argument, span));
      unified_param_types.push_back(
          const_cast<TypeAnnotation*>(unified_param_type));
    }
    return module_.Make<FunctionTypeAnnotation>(
        unified_param_types, const_cast<TypeAnnotation*>(unified_return_type));
  }

  // Unifies multiple annotations for a parametric struct, and produces an
  // annotation with agreeing explicit parametric values.
  absl::StatusOr<const TypeAnnotation*> UnifyParametricStructAnnotations(
      const StructDef& struct_def,
      std::vector<const TypeAnnotation*> annotations) {
    VLOG(6) << "Unifying parametric struct annotations; struct def: "
            << struct_def.identifier();
    std::vector<InterpValue> explicit_parametrics;
    std::optional<const StructInstance*> instantiator;

    // Go through the annotations, and check that they have no disagreement in
    // their explicit parametric values. For example, one annotation may be
    // `SomeStruct<32>` and one may be `SomeStruct<N>` where `N` is a parametric
    // of the enclosing function. We are in a position now to decide if `N` is
    // 32 or not.
    for (const TypeAnnotation* annotation : annotations) {
      VLOG(6) << "Annotation: " << annotation->ToString();
      XLS_ASSIGN_OR_RETURN(std::optional<StructOrProcRef> struct_or_proc_ref,
                           GetStructOrProcRef(annotation, file_table_));
      CHECK(struct_or_proc_ref.has_value());
      if (struct_or_proc_ref->instantiator.has_value()) {
        instantiator = struct_or_proc_ref->instantiator;
      }
      for (int i = 0; i < struct_or_proc_ref->parametrics.size(); i++) {
        ExprOrType parametric = struct_or_proc_ref->parametrics[i];
        const ParametricBinding* binding = struct_def.parametric_bindings()[i];
        CHECK(std::holds_alternative<Expr*>(parametric));
        auto* expr = std::get<Expr*>(parametric);
        XLS_ASSIGN_OR_RETURN(
            InterpValue value,
            evaluator_.Evaluate(ParametricContextScopedExpr(
                parametric_context_, binding->type_annotation(), expr)));
        if (i == explicit_parametrics.size()) {
          explicit_parametrics.push_back(value);
        } else if (value != explicit_parametrics[i]) {
          return absl::InvalidArgumentError(absl::Substitute(
              "Value mismatch for parametric `$0` of struct `$1`: $2 vs. $3",
              binding->identifier(), struct_def.identifier(), value.ToString(),
              explicit_parametrics[i].ToString()));
        }
      }
    }
    return parametric_struct_instantiator_.InstantiateParametricStruct(
        parametric_context_, struct_def, explicit_parametrics, instantiator);
  }

  absl::StatusOr<const TypeAnnotation*> UnifyBitsLikeTypeAnnotations(
      const std::vector<const TypeAnnotation*>& annotations, const Span& span) {
    std::optional<SignednessAndSize> unified_signedness_and_bit_count;
    for (int i = 0; i < annotations.size(); ++i) {
      const TypeAnnotation* current_annotation = annotations[i];
      VLOG(6) << "Annotation " << i << ": " << current_annotation->ToString();
      absl::StatusOr<SignednessAndBitCountResult> signedness_and_bit_count =
          GetSignednessAndBitCount(current_annotation);
      bool current_annotation_is_auto =
          table_.IsAutoLiteral(current_annotation);
      if (!signedness_and_bit_count.ok()) {
        return error_generator_.TypeMismatchError(
            parametric_context_, current_annotation, annotations[0]);
      }
      XLS_ASSIGN_OR_RETURN(
          bool current_annotation_signedness,
          evaluator_.EvaluateBoolOrExpr(parametric_context_,
                                        signedness_and_bit_count->signedness));
      XLS_ASSIGN_OR_RETURN(
          int64_t current_annotation_raw_bit_count,
          evaluator_.EvaluateU32OrExpr(parametric_context_,
                                       signedness_and_bit_count->bit_count));
      SignednessAndSize current_annotation_signedness_and_bit_count{
          .is_auto = current_annotation_is_auto,
          .is_signed = current_annotation_signedness,
          .size = current_annotation_raw_bit_count};

      XLS_ASSIGN_OR_RETURN(
          unified_signedness_and_bit_count,
          UnifySignednessAndSize(parametric_context_,
                                 unified_signedness_and_bit_count,
                                 current_annotation_signedness_and_bit_count,
                                 annotations[0], current_annotation));
      VLOG(6) << "Unified type so far has signedness: "
              << unified_signedness_and_bit_count->is_signed
              << " and bit count: " << unified_signedness_and_bit_count->size;
    }
    const TypeAnnotation* result = SignednessAndSizeToAnnotation(
        module_, *unified_signedness_and_bit_count, span);
    // An annotation we fabricate as a unification of a bunch of auto
    // annotations, is also considered an auto annotation itself.
    if (unified_signedness_and_bit_count->is_auto) {
      table_.MarkAsAutoLiteral(result);
    }
    return result;
  }

  // Returns a `SignednessAndSize` that agrees with the two given
  // `SignednessAndSize` objects if possible. `x` is optional for convenience of
  // invoking this in a loop where the first call has no preceding value.
  //
  // Any error returned is a size or signedness mismatch error suitable for
  // display to the user. The `parametric_context` and the passed in type
  // annotations are used only for the purpose of generating errors. It is
  // assumed that `y_annotation` should be mentioned first in errors.
  absl::StatusOr<SignednessAndSize> UnifySignednessAndSize(
      std::optional<const ParametricContext*> parametric_context,
      std::optional<SignednessAndSize> x, SignednessAndSize y,
      const TypeAnnotation* x_annotation, const TypeAnnotation* y_annotation) {
    if (!x.has_value()) {
      return y;
    }
    if (x->is_auto && y.is_auto) {
      return SignednessAndSize{
          .is_auto = true,
          .is_signed = x->is_signed || y.is_signed,
          // If we are coercing one of 2 auto annotations to signed, the one
          // being coerced needs an extra bit to keep fitting the value it was
          // sized to.
          .size = x->size == y.size && x->is_signed != y.is_signed
                      ? x->size + 1
                      : std::max(x->size, y.size)};
    }
    // Converts `annotation` into one that reflects `signedness_and_size`, for
    // error purposes, if it is auto. If it is explicit, then we would not have
    // modified `signedness_and_size`, and we want to display `annotation`'s
    // original formulation, which may not be canonical. Note that
    // `signedness_and_size` may have been modified by a prior call to
    // `UnifySignednessAndSize`, and not necessarily the current call.
    auto update_annotation = [&](const SignednessAndSize& signedness_and_size,
                                 const TypeAnnotation* annotation) {
      return signedness_and_size.is_auto
                 ? SignednessAndSizeToAnnotation(module_, signedness_and_size,
                                                 annotation->span())
                 : annotation;
    };
    auto signedness_mismatch_error = [&] {
      return error_generator_.SignednessMismatchError(
          parametric_context, update_annotation(y, y_annotation),
          update_annotation(*x, x_annotation));
    };
    auto bit_count_mismatch_error = [&] {
      return error_generator_.BitCountMismatchError(
          parametric_context, update_annotation(y, y_annotation),
          update_annotation(*x, x_annotation));
    };
    if (x->is_auto || y.is_auto) {
      SignednessAndSize& auto_value = x->is_auto ? *x : y;
      SignednessAndSize& explicit_value = x->is_auto ? y : *x;
      if (auto_value.is_signed && !explicit_value.is_signed) {
        return signedness_mismatch_error();
      }
      if (!auto_value.is_signed && explicit_value.is_signed) {
        // An auto value being coerced to be signed needs to be extended for the
        // same reason as above.
        auto_value.is_signed = true;
        ++auto_value.size;
      }
      if (explicit_value.size >= auto_value.size) {
        return explicit_value;
      }
      return bit_count_mismatch_error();
    }
    // They are both explicit and must match.
    if (x->size != y.size) {
      return bit_count_mismatch_error();
    }
    if (x->is_signed != y.is_signed) {
      return signedness_mismatch_error();
    }
    return *x;
  }

  Module& module_;
  InferenceTable& table_;
  const FileTable& file_table_;
  UnificationErrorGenerator& error_generator_;
  Evaluator& evaluator_;
  ParametricStructInstantiator& parametric_struct_instantiator_;
  IndirectAnnotationResolver& indirect_annotation_resolver_;
  std::optional<const ParametricContext*> parametric_context_;
  std::optional<absl::FunctionRef<bool(const TypeAnnotation*)>>
      accept_predicate_;
};

}  // namespace

absl::StatusOr<const TypeAnnotation*> UnifyTypeAnnotations(
    Module& module, InferenceTable& table, const FileTable& file_table,
    UnificationErrorGenerator& error_generator, Evaluator& evaluator,
    ParametricStructInstantiator& parametric_struct_instantiator,
    IndirectAnnotationResolver& indirect_annotation_resolver,
    std::optional<const ParametricContext*> parametric_context,
    std::vector<const TypeAnnotation*> annotations, const Span& span,
    std::optional<absl::FunctionRef<bool(const TypeAnnotation*)>>
        accept_predicate) {
  Unifier unifier(module, table, file_table, error_generator, evaluator,
                  parametric_struct_instantiator, indirect_annotation_resolver,
                  parametric_context, accept_predicate);
  return unifier.UnifyTypeAnnotations(annotations, span);
}

}  // namespace xls::dslx
