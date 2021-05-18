// Copyright 2020 The XLS Authors
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

#ifndef XLS_IR_FUNCTION_BUILDER_H_
#define XLS_IR_FUNCTION_BUILDER_H_

// NOTE: We're trying to keep the API minimal here to not drag too much into
// publicly-exposed APIs (FunctionBuilder/ProcBuilder are publicly exposed), so
// forward decls are used.
//
// To this end, DO NOT add node/function headers here.

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "xls/common/logging/logging.h"
#include "xls/common/status/ret_check.h"
#include "xls/ir/block.h"
#include "xls/ir/function.h"
#include "xls/ir/lsb_or_msb.h"
#include "xls/ir/op.h"
#include "xls/ir/package.h"
#include "xls/ir/proc.h"
#include "xls/ir/source_location.h"
#include "xls/ir/value.h"

namespace xls {

class BuilderBase;
class Channel;
class Function;
class FunctionBase;
class Node;
class Proc;
class Type;

// Represents a value for use in the function-definition building process,
// supports some basic C++ operations that have a natural correspondence to the
// staged versions.
class BValue {
 public:
  // The vacuous constructor lets us use values in vectors and as values that
  // are subsequently initialized. Check valid() to ensure proper
  // initialization.
  BValue() : BValue(nullptr, nullptr) {}

  BValue(Node* node, BuilderBase* builder) : node_(node), builder_(builder) {}

  BuilderBase* builder() const { return builder_; }

  Node* node() const { return node_; }
  Type* GetType() const;
  int64_t BitCountOrDie() const;
  absl::optional<SourceLocation> loc() const;
  std::string ToString() const;

  bool valid() const {
    XLS_CHECK_EQ(node_ == nullptr, builder_ == nullptr);
    return node_ != nullptr;
  }

  // Sets the name of the node.
  std::string SetName(absl::string_view name);

  // Returns the name of the node.
  std::string GetName() const;

  // Returns whether the node has been assigned a name. Nodes without assigned
  // names have names generated from the opcode and unique id.
  bool HasAssignedName() const;

  BValue operator>>(BValue rhs);
  BValue operator<<(BValue rhs);
  BValue operator|(BValue rhs);
  BValue operator-(BValue rhs);
  BValue operator+(BValue rhs);
  BValue operator*(BValue rhs);
  BValue operator^(BValue rhs);

  // Note: unary negation.
  BValue operator-();

 private:
  Node* node_;
  BuilderBase* builder_;
};

// Base class for function and proc. Provides interface for adding nodes to a
// function proc. Example usage (for derived FunctionBuilder class):
//
//  Package p("my_package");
//  FunctionBuilder b("my_or_function_32b", &p);
//  auto bits_32 = m.GetBitsType(32);
//  auto x = b.Param("x", bits_32);
//  auto y = b.Param("y", bits_32);
//  XLS_ASSIGN_OR_RETURN(Function* f, b.BuildWithReturnValue(x | y));
//
// Note that the BValues returned by builder routines are specific to that
// function, and attempt to use those BValues with FunctionBuilders that did not
// originate them will cause fatal errors.
class BuilderBase {
 public:
  // The given argument 'function' can contain either a Function or Proc.
  // 'should_verify' is a test-only argument which can be set to false in tests
  // that wish to build malformed IR.
  explicit BuilderBase(std::unique_ptr<FunctionBase> function,
                       bool should_verify = true);
  virtual ~BuilderBase();

  const std::string& name() const;

  // Get access to currently built up function (or proc).
  FunctionBase* function() const { return function_.get(); }

  // Declares a parameter to the function being built of type "type".
  virtual BValue Param(absl::string_view name, Type* type,
                       absl::optional<SourceLocation> loc = absl::nullopt) = 0;

  // Shift right arithmetic.
  BValue Shra(BValue operand, BValue amount,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");

  // Shift right logical.
  BValue Shrl(BValue operand, BValue amount,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");

  // Shift left (logical).
  BValue Shll(BValue operand, BValue amount,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");

  // Bitwise or.
  BValue Or(BValue lhs, BValue rhs,
            absl::optional<SourceLocation> loc = absl::nullopt,
            absl::string_view name = "");
  BValue Or(absl::Span<const BValue> operands,
            absl::optional<SourceLocation> loc = absl::nullopt,
            absl::string_view name = "");

  // Bitwise xor.
  BValue Xor(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");
  BValue Xor(absl::Span<const BValue> operands,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");

  // Bitwise and.
  BValue And(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");
  BValue And(absl::Span<const BValue> operands,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");

  // Unary and-reduction.
  BValue AndReduce(BValue operand,
                   absl::optional<SourceLocation> loc = absl::nullopt,
                   absl::string_view name = "");

  // Unary or-reduction.
  BValue OrReduce(BValue operand,
                  absl::optional<SourceLocation> loc = absl::nullopt,
                  absl::string_view name = "");

  // Unary xor-reduction.
  BValue XorReduce(BValue operand,
                   absl::optional<SourceLocation> loc = absl::nullopt,
                   absl::string_view name = "");

  // Unsigned/signed multiply. Result width is the same as the operand
  // width. Operand widths must be the same.
  BValue UMul(BValue lhs, BValue rhs,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");
  BValue SMul(BValue lhs, BValue rhs,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");

  // Unsigned/signed multiply with explicitly specified result width. Operand
  // widths can be arbitrary.
  BValue UMul(BValue lhs, BValue rhs, int64_t result_width,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");
  BValue SMul(BValue lhs, BValue rhs, int64_t result_width,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");

  // Unsigned/signed division. Result width is the same as the operand
  // width. Operand widths must be the same.
  BValue UDiv(BValue lhs, BValue rhs,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");
  BValue SDiv(BValue lhs, BValue rhs,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");

  // Two's complement subtraction/addition.
  BValue Subtract(BValue lhs, BValue rhs,
                  absl::optional<SourceLocation> loc = absl::nullopt,
                  absl::string_view name = "");
  BValue Add(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");

  // Concatenates the operands (zero-th operand are the most significant bits in
  // the result).
  BValue Concat(absl::Span<const BValue> operands,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // Unsigned less-or-equal.
  BValue ULe(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");
  // Unsigned less-than.
  BValue ULt(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");
  // Unsigned greater-or-equal.
  BValue UGe(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");
  // Unsigned greater-than.
  BValue UGt(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");

  // Signed less-or-equal.
  BValue SLe(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");
  // Signed less-than.
  BValue SLt(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");
  // Signed greater-or-equal.
  BValue SGe(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");
  // Signed greater-than.
  BValue SGt(BValue lhs, BValue rhs,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");

  // Equal.
  BValue Eq(BValue lhs, BValue rhs,
            absl::optional<SourceLocation> loc = absl::nullopt,
            absl::string_view name = "");
  // Not Equal.
  BValue Ne(BValue lhs, BValue rhs,
            absl::optional<SourceLocation> loc = absl::nullopt,
            absl::string_view name = "");

  // Two's complement (arithmetic) negation.
  BValue Negate(BValue x, absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // One's complement negation (bitwise not).
  BValue Not(BValue x, absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");

  // Turns a literal value into a handle that can be used in this function being
  // built.
  BValue Literal(Value value,
                 absl::optional<SourceLocation> loc = absl::nullopt,
                 absl::string_view name = "");
  BValue Literal(Bits bits, absl::optional<SourceLocation> loc = absl::nullopt,
                 absl::string_view name = "") {
    return Literal(Value(bits), loc);
  }

  // An n-ary select which selects among an arbitrary number of cases based on
  // the selector value. If the number of cases is less than 2**selector_width
  // then a default value must be specified.
  BValue Select(BValue selector, absl::Span<const BValue> cases,
                absl::optional<BValue> default_value = absl::nullopt,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // An overload for binary select: selects on_true when selector is true,
  // on_false otherwise.
  // TODO(meheff): switch positions of on_true and on_false to match the
  // ordering of the cases span in the n-ary select.
  BValue Select(BValue selector, BValue on_true, BValue on_false,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // Creates a one-hot operation which generates a one-hot bits value from its
  // inputs.
  BValue OneHot(BValue input, LsbOrMsb priority,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // Creates a select operation which uses a one-hot selector (rather than a
  // binary encoded selector as used in Select).
  BValue OneHotSelect(BValue selector, absl::Span<const BValue> cases,
                      absl::optional<SourceLocation> loc = absl::nullopt,
                      absl::string_view name = "");

  // Counts the number of leading zeros in the value 'x'.
  //
  // x must be of bits type. This function returns a value of the same type that
  // it takes as an argument.
  BValue Clz(BValue x, absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");

  // Counts the number of trailing zeros in the value 'x'.
  //
  // x must be of bits type. This function returns a value of the same type that
  // it takes as an argument.
  BValue Ctz(BValue x, absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");

  // Creates an match operation which compares 'condition' against each of the
  // case clauses and produces the respective case value of the first match. If
  // 'condition' matches no cases 'default_value' is produced. 'condition' and
  // each case clause must be of the same type. 'default_value' and each case
  // value must be of the same type. The match operation is composed of multiple
  // IR nodes including one-hot and one-hot-select.
  struct Case {
    BValue clause;
    BValue value;
  };
  BValue Match(BValue condition, absl::Span<const Case> cases,
               BValue default_value,
               absl::optional<SourceLocation> loc = absl::nullopt,
               absl::string_view name = "");

  // Creates a match operation which matches each of the case clauses against
  // the single-bit literal one. Each case clause must be a single-bit Bits
  // value.
  BValue MatchTrue(absl::Span<const Case> cases, BValue default_value,
                   absl::optional<SourceLocation> loc = absl::nullopt,
                   absl::string_view name = "");
  // Overload which is easier to expose to Python.
  BValue MatchTrue(absl::Span<const BValue> case_clauses,
                   absl::Span<const BValue> case_values, BValue default_value,
                   absl::optional<SourceLocation> loc = absl::nullopt,
                   absl::string_view name = "");

  // Creates a tuple of values.
  //
  // Note that "loc" would generally refer to the source location whether the
  // tuple expression begins; e.g. in DSLX:
  //
  //    let x = (1, 2, 3, 4) in ...
  //  ~~~~~~~~~~^
  //
  //  The arrow indicates the source location for the tuple expression.
  BValue Tuple(absl::Span<const BValue> elements,
               absl::optional<SourceLocation> loc = absl::nullopt,
               absl::string_view name = "");

  // Creates an AfterAll ordering operation.
  BValue AfterAll(absl::Span<const BValue> dependencies,
                  absl::optional<SourceLocation> loc = absl::nullopt,
                  absl::string_view name = "");

  // Creates an array of values. Each value in element must be the same type
  // which is given by element_type.
  BValue Array(absl::Span<const BValue> elements, Type* element_type,
               absl::optional<SourceLocation> loc = absl::nullopt,
               absl::string_view name = "");

  // Adds an tuple index expression.
  BValue TupleIndex(BValue arg, int64_t idx,
                    absl::optional<SourceLocation> loc = absl::nullopt,
                    absl::string_view name = "");

  // Adds a "counted for-loop" to the computation, having a known-constant
  // number of loop iterations.
  //
  // Args:
  //  trip_count: Number of iterations to execute "body".
  //  init_value: Of type "T", the starting value for the loop carry data.
  //  body: Of type "(u32, T) -> T": the body that is invoked on the loop carry
  //    data decorated with the iteration number, producing new loop carry data
  //    (of the same type) each iteration.
  //  invariant_args: Arguments that are passed to the body function in a
  //    loop-invariant fashion (for each of these arguments, the same value is
  //    passed as an argument on every trip and the value does not change).
  //  loc: Source location for this counted for-loop.
  //
  // Returns the value that results from this counted for loop after it has
  // completed all of its trips.
  BValue CountedFor(BValue init_value, int64_t trip_count, int64_t stride,
                    Function* body,
                    absl::Span<const BValue> invariant_args = {},
                    absl::optional<SourceLocation> loc = absl::nullopt,
                    absl::string_view name = "");

  // Adds a "dynamic counted for-loop" to the computation, having a
  // dynamically determined number of loop iterations and stride.
  //
  // Args:
  //  trip_count: Of bits type, number of iterations to execute "body".
  //  stride: Of bits type, count by which to increment induction variable.
  //  init_value: Of type "T", the starting value for the loop carry data.
  //  body: Of type "(u32, T) -> T": the body that is invoked on the loop carry
  //    data decorated with the iteration number, producing new loop carry data
  //    (of the same type) each iteration.
  //  invariant_args: Arguments that are passed to the body function in a
  //    loop-invariant fashion (for each of these arguments, the same value is
  //    passed as an argument on every trip and the value does not change).
  //  loc: Source location for this counted for-loop.
  //
  // Returns the value that results from this counted for loop after it has
  // completed all of its trips.
  BValue DynamicCountedFor(BValue init_value, BValue trip_count, BValue stride,
                           Function* body,
                           absl::Span<const BValue> invariant_args = {},
                           absl::optional<SourceLocation> loc = absl::nullopt,
                           absl::string_view name = "");

  // Adds a map to the computation.
  // Applies the function to_apply to each element of the array-typed operand
  // and returns the result as an array
  //
  // Args:
  //   operand: Of type "Array<T, N>" containing N elements of type T.
  //   to_apply: Of type "T -> U".
  //   loc: Source location for this map.
  //
  // Returns a value of type "Array<U, N>".
  BValue Map(BValue operand, Function* to_apply,
             absl::optional<SourceLocation> loc = absl::nullopt,
             absl::string_view name = "");

  // Adds a function call with parameters.
  BValue Invoke(absl::Span<const BValue> args, Function* to_apply,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // Adds an multi-dimensional array index expression. The indices should be all
  // bits types.
  BValue ArrayIndex(BValue arg, absl::Span<const BValue> indices,
                    absl::optional<SourceLocation> loc = absl::nullopt,
                    absl::string_view name = "");

  // Slices an array with a given start and end position.
  BValue ArraySlice(BValue array, BValue start, int64_t width,
                    absl::optional<SourceLocation> loc = absl::nullopt,
                    absl::string_view name = "");

  // Updates the array element at index "idx" to update_value. The indices
  // should be all bits types.
  BValue ArrayUpdate(BValue arg, BValue update_value,
                     absl::Span<const BValue> indices,
                     absl::optional<SourceLocation> loc = absl::nullopt,
                     absl::string_view name = "");

  // Concatenates array operands into a single array.  zero-th
  // element is the zero-th element of the zero-th (left-most) array.
  BValue ArrayConcat(absl::Span<const BValue> operands,
                     absl::optional<SourceLocation> loc = absl::nullopt,
                     absl::string_view name = "");

  // Reverses the order of the bits of the argument.
  BValue Reverse(BValue arg, absl::optional<SourceLocation> loc = absl::nullopt,
                 absl::string_view name = "");

  // Adds an identity expression.
  BValue Identity(BValue var,
                  absl::optional<SourceLocation> loc = absl::nullopt,
                  absl::string_view name = "");

  // Sign-extends arg to the new_bit_count.
  BValue SignExtend(BValue arg, int64_t new_bit_count,
                    absl::optional<SourceLocation> loc = absl::nullopt,
                    absl::string_view name = "");

  // Zero-extends arg to the new_bit_count.
  BValue ZeroExtend(BValue arg, int64_t new_bit_count,
                    absl::optional<SourceLocation> loc = absl::nullopt,
                    absl::string_view name = "");

  // Extracts a slice from the bits-typed arg. 'start' is the first bit of the
  // slice and is zero-indexed where zero is the LSb of arg.
  BValue BitSlice(BValue arg, int64_t start, int64_t width,
                  absl::optional<SourceLocation> loc = absl::nullopt,
                  absl::string_view name = "");

  // Updates a slice of the given bits-typed 'arg' starting at index 'start'
  // with 'update_value'.
  BValue BitSliceUpdate(BValue arg, BValue start, BValue update_value,
                        absl::optional<SourceLocation> loc = absl::nullopt,
                        absl::string_view name = "");

  // Same as BitSlice, but allows for dynamic 'start' offsets
  BValue DynamicBitSlice(BValue arg, BValue start, int64_t width,
                         absl::optional<SourceLocation> loc = absl::nullopt,
                         absl::string_view name = "");

  // Binary encodes the n-bit input to a log_2(n) bit output.
  BValue Encode(BValue arg, absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // Binary decodes the n-bit input to a one-hot output. 'width' can be at most
  // 2**n where n is the bit width of the operand. If 'width' is not specified
  // the output is 2**n bits wide.
  BValue Decode(BValue arg, absl::optional<int64_t> width = absl::nullopt,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // Retrieves the type of "value" and returns it.
  Type* GetType(const BValue& value) { return value.GetType(); }

  // Adds a Arith/UnOp/BinOp/CompareOp to the function. Exposed for
  // programmatically adding these ops using with variable Op values.
  BValue AddUnOp(Op op, BValue x,
                 absl::optional<SourceLocation> loc = absl::nullopt,
                 absl::string_view name = "");
  BValue AddBinOp(Op op, BValue lhs, BValue rhs,
                  absl::optional<SourceLocation> loc = absl::nullopt,
                  absl::string_view name = "");
  BValue AddCompareOp(Op op, BValue lhs, BValue rhs,
                      absl::optional<SourceLocation> loc = absl::nullopt,
                      absl::string_view name = "");
  BValue AddNaryOp(Op op, absl::Span<const BValue> args,
                   absl::optional<SourceLocation> loc = absl::nullopt,
                   absl::string_view name = "");
  // If result width is not given the result width set to the width of the
  // arguments lhs and rhs which must have the same width.
  BValue AddArithOp(Op op, BValue lhs, BValue rhs,
                    absl::optional<int64_t> result_width,
                    absl::optional<SourceLocation> loc = absl::nullopt,
                    absl::string_view name = "");
  BValue AddBitwiseReductionOp(
      Op op, BValue arg, absl::optional<SourceLocation> loc = absl::nullopt,
      absl::string_view name = "");

  // Adds an assert op to the function. Assert raises an error containing the
  // given message if the given condition evaluates to false.
  BValue Assert(BValue token, BValue condition, absl::string_view message,
                absl::optional<std::string> label = absl::nullopt,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // Add a receive operation. The type of the data value received is
  // determined by the channel. Only supported on procs.
  virtual BValue Receive(Channel* channel, BValue token,
                         absl::optional<SourceLocation> loc = absl::nullopt,
                         absl::string_view name = "");

  // Add a receive_if operation. The receive executes conditionally on the value
  // of the predicate "pred". The type of the data value received is
  // determined by the channel. Only supported on procs.
  virtual BValue ReceiveIf(Channel* channel, BValue token, BValue pred,
                           absl::optional<SourceLocation> loc = absl::nullopt,
                           absl::string_view name = "");

  // Add a send operation. Only supported on procs.
  virtual BValue Send(Channel* channel, BValue token, BValue data,
                      absl::optional<SourceLocation> loc = absl::nullopt,
                      absl::string_view name = "");

  // Add a send_if operation. The send executes conditionally on the value of
  // the predicate "pred". Only supported on procs.
  virtual BValue SendIf(Channel* channel, BValue token, BValue pred,
                        BValue data,
                        absl::optional<SourceLocation> loc = absl::nullopt,
                        absl::string_view name = "");

  // Add an input/output port. Only supported on blocks.
  virtual BValue InputPort(absl::string_view name, Type* type,
                           absl::optional<SourceLocation> loc = absl::nullopt);
  virtual BValue OutputPort(absl::string_view name, BValue operand,
                            absl::optional<SourceLocation> loc = absl::nullopt);

  Package* package() const;

  // Returns the last node enqueued onto this builder -- when Build() is called
  // this is what is used as the return value.
  absl::StatusOr<BValue> GetLastValue() {
    XLS_RET_CHECK(last_node_ != nullptr);
    return BValue(last_node_, this);
  }

 protected:
  BValue SetError(absl::string_view msg, absl::optional<SourceLocation> loc);
  bool ErrorPending() { return error_pending_; }

  // Constructs and adds a node to the function and returns a corresponding
  // BValue.
  template <typename NodeT, typename... Args>
  BValue AddNode(absl::optional<SourceLocation> loc, Args&&... args);

  BValue CreateBValue(Node* node, absl::optional<SourceLocation> loc);

  // The most recently added node to the function.
  Node* last_node_ = nullptr;

  // The function being built by this builder.
  std::unique_ptr<FunctionBase> function_;

  bool error_pending_;

  // Whether nodes and the built function should be verified. Only false in
  // tests.
  bool should_verify_;

  std::string error_msg_;
  std::string error_stacktrace_;
  absl::optional<SourceLocation> error_loc_;
};

// Class for building an XLS Function.
class FunctionBuilder : public BuilderBase {
 public:
  // Builder for xls::Functions. 'should_verify' is a test-only argument which
  // can be set to false in tests that wish to build malformed IR.
  FunctionBuilder(absl::string_view name, Package* package,
                  bool should_verify = true);
  virtual ~FunctionBuilder() = default;

  BValue Param(absl::string_view name, Type* type,
               absl::optional<SourceLocation> loc = absl::nullopt) override;

  // Adds the function internally being built-up by this builder to the package
  // given at construction time, and returns a pointer to it (the function is
  // subsequently owned by the package and this builder should be discarded).
  //
  // The return value of the function is the most recently added operation.
  absl::StatusOr<Function*> Build();

  // Build function using given return value.
  absl::StatusOr<Function*> BuildWithReturnValue(BValue return_value);
};

// Class for building an XLS Proc (a communicating sequential process).
class ProcBuilder : public BuilderBase {
 public:
  // Builder for xls::Procs. 'should_verify' is a test-only argument which can
  // be set to false in tests that wish to build malformed IR.
  ProcBuilder(absl::string_view name, const Value& init_value,
              absl::string_view token_name, absl::string_view state_name,
              Package* package, bool should_verify = true);
  virtual ~ProcBuilder() = default;

  // Returns the Proc being constructed.
  Proc* proc() const;

  // Returns the Param BValue for the state or token parameter. Unlike
  // BuilderBase::Param this does add a Param node to the Proc. Rather the state
  // and token parameters are added to the Proc at construction time and these
  // methods return references to these parameters.
  BValue GetStateParam() const { return state_param_; }
  BValue GetTokenParam() const { return token_param_; }

  // Build the proc using the given BValues as the recurrent token and state
  // respectively.
  absl::StatusOr<Proc*> Build(BValue token, BValue next_state);

  BValue Param(absl::string_view name, Type* type,
               absl::optional<SourceLocation> loc = absl::nullopt) override;
  BValue Receive(Channel* channel, BValue token,
                 absl::optional<SourceLocation> loc = absl::nullopt,
                 absl::string_view name = "") override;
  BValue ReceiveIf(Channel* channel, BValue token, BValue pred,
                   absl::optional<SourceLocation> loc = absl::nullopt,
                   absl::string_view name = "") override;
  BValue Send(Channel* channel, BValue token, BValue data,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "") override;
  BValue SendIf(Channel* channel, BValue token, BValue pred, BValue data,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "") override;

 private:
  // The BValue of the token parameter (parameter 0).
  BValue token_param_;

  // The BValue of the state parameter (parameter 1).
  BValue state_param_;
};

// A derived ProcBuilder which automatically manages tokens internally.  This
// makes it much less verbose to construct procs with sends, receives, or other
// token-using operations. In the TokenlessProcBuilder, *all* token-using
// operations use GetTokenParam() as their token operand, and the recurrent
// token is an AfterAll operation whose operands are the tokens from all of the
// token-using operations. The limitation of the TokenlessProcBuilder is that it
// cannot be used if non-trivial ordering of these operations is required
// (enforced via token data dependencies).
//
// Note: a proc built with the TokenlessProcBuilder still has token types
// internally. "Tokenless" refers to the fact that token values are hidden from
// the builder interface.
class TokenlessProcBuilder : public ProcBuilder {
 public:
  TokenlessProcBuilder(absl::string_view name, const Value& init_value,
                       absl::string_view token_name,
                       absl::string_view state_name, Package* package,
                       bool should_verify = true)
      : ProcBuilder(name, init_value, token_name, state_name, package,
                    should_verify) {}
  virtual ~TokenlessProcBuilder() = default;

  // Build the proc using the given BValue as the recurrent state
  // respectively. The recurrent token value is a constructed as an AfterAll
  // operation whose operands are all of the tokens from the
  // send(if)/receive(if) operations in the proc.
  absl::StatusOr<Proc*> Build(BValue next_state);

  // Add a receive operation. The type of the data value received is determined
  // by the channel. The returned BValue is the received data itself (*not* the
  // receive operation itself which produces a tuple containing a token and the
  // data).
  using ProcBuilder::Receive;
  BValue Receive(Channel* channel,
                 absl::optional<SourceLocation> loc = absl::nullopt,
                 absl::string_view name = "");

  // Add a receive_if operation. The receive executes conditionally on the value
  // of the predicate "pred". The type of the data value received is determined
  // by the channel.  The returned BValue is the received data itself (*not* the
  // receiveif operation itself which produces a tuple containing a token and
  // the data).
  using ProcBuilder::ReceiveIf;
  BValue ReceiveIf(Channel* channel, BValue pred,
                   absl::optional<SourceLocation> loc = absl::nullopt,
                   absl::string_view name = "");

  // Add a send operation. Returns the token-typed BValue of the send node.
  using ProcBuilder::Send;
  BValue Send(Channel* channel, BValue data,
              absl::optional<SourceLocation> loc = absl::nullopt,
              absl::string_view name = "");

  // Add a send_if operation. The send executes conditionally on the value of
  // the predicate "pred". Returns the token-typed BValue of the send_if node.
  using ProcBuilder::SendIf;
  BValue SendIf(Channel* channel, BValue pred, BValue data,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

  // Add an assert operation. Returns the token-typed assert operation.
  using BuilderBase::Assert;
  BValue Assert(BValue condition, absl::string_view message,
                absl::optional<std::string> label = absl::nullopt,
                absl::optional<SourceLocation> loc = absl::nullopt,
                absl::string_view name = "");

 private:
  // The tokens from any added send(if)/receive(if) nodes.
  std::vector<BValue> tokens_;
};

// Class for building an XLS Block.
class BlockBuilder : public BuilderBase {
 public:
  // Builder for xls::Blocks. 'should_verify' is a test-only argument which can
  // be set to false in tests that wish to build malformed IR.
  BlockBuilder(absl::string_view name, Package* package,
               bool should_verify = true)
      : BuilderBase(absl::make_unique<Block>(name, package), should_verify) {}
  virtual ~BlockBuilder() = default;

  // Returns the Block being constructed.
  Block* block() const { return down_cast<Block*>(function()); }

  // Build the block.
  absl::StatusOr<Block*> Build();

  BValue Param(absl::string_view name, Type* type,
               absl::optional<SourceLocation> loc = absl::nullopt) override;

  BValue InputPort(absl::string_view name, Type* type,
                   absl::optional<SourceLocation> loc = absl::nullopt) override;
  BValue OutputPort(
      absl::string_view name, BValue operand,
      absl::optional<SourceLocation> loc = absl::nullopt) override;
};

}  // namespace xls

#endif  // XLS_IR_FUNCTION_BUILDER_H_
