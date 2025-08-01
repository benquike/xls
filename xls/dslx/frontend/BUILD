# Copyright 2023 The XLS Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Front-end types and routines for e.g. parsing/scanning/AST.

load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

package(
    default_applicable_licenses = ["//:license"],
    default_visibility = ["//xls:xls_internal"],
    features = [
        "layering_check",
        "parse_headers",
    ],
    licenses = ["notice"],  # Apache 2.0
)

cc_library(
    name = "ast_cloner",
    srcs = ["ast_cloner.cc"],
    hdrs = ["ast_cloner.h"],
    deps = [
        ":ast",
        ":ast_utils",
        ":module",
        ":pos",
        ":proc",
        "//xls/common:casts",
        "//xls/common:visitor",
        "//xls/common/status:status_macros",
        "//xls/ir:format_strings",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/functional:any_invocable",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "ast_cloner_test",
    srcs = ["ast_cloner_test.cc"],
    deps = [
        ":ast",
        ":ast_cloner",
        ":ast_utils",
        ":module",
        ":pos",
        "//xls/common:casts",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/common/status:status_macros",
        "//xls/dslx:command_line_utils",
        "//xls/dslx:parse_and_typecheck",
        "//xls/dslx:virtualizable_file_system",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "token_parser",
    srcs = ["token_parser.cc"],
    hdrs = ["token_parser.h"],
    deps = [
        ":bindings",
        ":pos",
        ":scanner",
        ":token",
        "//xls/common/status:status_macros",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":ast",
        ":ast_cloner",
        ":ast_utils",
        ":bindings",
        ":builtins_metadata",
        ":module",
        ":pos",
        ":proc",
        ":scanner",
        ":token",
        ":token_parser",
        "//xls/common:casts",
        "//xls/common:strong_int",
        "//xls/common:visitor",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_builder",
        "//xls/common/status:status_macros",
        "//xls/dslx:channel_direction",
        "//xls/ir:channel",
        "//xls/ir:code_template",
        "//xls/ir:foreign_function",
        "//xls/ir:foreign_function_data_cc_proto",
        "//xls/ir:format_strings",
        "//xls/ir:name_uniquer",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/base:nullability",
        "@com_google_absl//absl/cleanup",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/functional:bind_front",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "parser_test",
    srcs = ["parser_test.cc"],
    data = ["builtin_stubs.x"],
    deps = [
        ":ast",
        ":bindings",
        ":builtins_metadata",
        ":module",
        ":parser",
        ":pos",
        ":scanner",
        "//xls/common:casts",
        "//xls/common:xls_gunit_main",
        "//xls/common/file:filesystem",
        "//xls/common/file:get_runfile_path",
        "//xls/common/status:matchers",
        "//xls/dslx:command_line_utils",
        "//xls/dslx:error_test_utils",
        "//xls/dslx:virtualizable_file_system",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/types:span",
        "@googletest//:gtest",
    ],
)

# Note: ast_utils layers on top of the AST implementation.
cc_library(
    name = "ast_utils",
    srcs = ["ast_utils.cc"],
    hdrs = ["ast_utils.h"],
    deps = [
        ":ast",
        ":builtins_metadata",
        ":module",
        ":pos",
        ":token_utils",
        "//xls/common:casts",
        "//xls/common:visitor",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx:interp_value",
        "//xls/ir:bits",
        "//xls/ir:number_parser",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "ast_utils_test",
    srcs = ["ast_utils_test.cc"],
    deps = [
        ":ast",
        ":ast_utils",
        ":module",
        ":parser",
        ":pos",
        ":scanner",
        "//xls/common:casts",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/dslx:parse_and_typecheck",
        "@com_google_absl//absl/log",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "token_utils",
    srcs = [
        "scanner_keywords.inc",
        "token_utils.cc",
    ],
    hdrs = ["token_utils.h"],
    deps = [
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "token_utils_test",
    srcs = ["token_utils_test.cc"],
    deps = [
        ":token_utils",
        "//xls/common:xls_gunit_main",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "token",
    srcs = ["token.cc"],
    hdrs = [
        "scanner_keywords.inc",
        "token.h",
    ],
    deps = [
        ":pos",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "token_test",
    srcs = ["token_test.cc"],
    deps = [
        ":pos",
        ":token",
        "//xls/common:xls_gunit_main",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "scanner",
    srcs = ["scanner.cc"],
    hdrs = ["scanner.h"],
    deps = [
        ":comment_data",
        ":pos",
        ":token",
        "//xls/common:test_macros",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/base:no_destructor",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "scanner_test",
    srcs = ["scanner_test.cc"],
    deps = [
        ":comment_data",
        ":pos",
        ":scanner",
        ":token",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/dslx:error_test_utils",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_absl//absl/status:statusor",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "bindings",
    srcs = ["bindings.cc"],
    hdrs = ["bindings.h"],
    deps = [
        ":ast",
        ":pos",
        "//xls/common:visitor",
        "//xls/common/status:status_macros",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:variant",
        "@re2",
    ],
)

cc_test(
    name = "bindings_test",
    srcs = ["bindings_test.cc"],
    deps = [
        ":ast",
        ":bindings",
        ":pos",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "builtins_metadata",
    srcs = ["builtins_metadata.cc"],
    hdrs = ["builtins_metadata.h"],
    deps = [
        "@com_google_absl//absl/base:no_destructor",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_library(
    name = "pos",
    srcs = ["pos.cc"],
    hdrs = ["pos.h"],
    deps = [
        "//xls/common:strong_int",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@re2",
    ],
)

cc_test(
    name = "pos_test",
    srcs = ["pos_test.cc"],
    deps = [
        ":pos",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "ast_builtin_types",
    hdrs = ["ast_builtin_types.inc"],
)

cc_library(
    name = "ast_node",
    srcs = ["ast_node.cc"],
    hdrs = ["ast_node.h"],
    deps = [
        ":pos",
        "//xls/common/status:status_macros",
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "ast",
    srcs = ["ast.cc"],
    hdrs = ["ast.h"],
    deps = [
        ":ast_builtin_types",
        ":ast_node",
        ":pos",
        ":token_utils",
        "//xls/common:casts",
        "//xls/common:indent",
        "//xls/common:visitor",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx:channel_direction",
        "//xls/ir:bits",
        "//xls/ir:bits_ops",
        "//xls/ir:channel",
        "//xls/ir:foreign_function_data_cc_proto",
        "//xls/ir:format_preference",
        "//xls/ir:format_strings",
        "//xls/ir:number_parser",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/base:no_destructor",
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_library(
    name = "ast_node_visitor_with_default",
    hdrs = ["ast_node_visitor_with_default.h"],
    deps = [
        ":ast",
        ":ast_node",
        ":module",
        ":proc",
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "proc",
    srcs = ["proc.cc"],
    hdrs = ["proc.h"],
    deps = [
        ":ast",
        ":ast_node",
        ":pos",
        "//xls/common:indent",
        "@com_google_absl//absl/base:nullability",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "module",
    srcs = ["module.cc"],
    hdrs = ["module.h"],
    deps = [
        ":ast",
        ":pos",
        ":proc",
        "//xls/common:casts",
        "//xls/common:visitor",
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_library(
    name = "ast_test_utils",
    testonly = True,
    srcs = ["ast_test_utils.cc"],
    hdrs = ["ast_test_utils.h"],
    deps = [
        ":ast",
        ":module",
        ":pos",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "ast_test",
    srcs = ["ast_test.cc"],
    deps = [
        ":ast",
        ":ast_test_utils",
        ":module",
        ":pos",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "comment_data",
    hdrs = ["comment_data.h"],
    deps = [
        ":pos",
    ],
)

cc_test(
    name = "scanned_token_to_string_fuzz_test",
    srcs = ["scanned_token_to_string_fuzz_test.cc"],
    tags = ["componentid:1408685"],
    deps = [
        ":bindings",
        ":pos",
        ":scanner",
        ":token",
        "//xls/common:xls_gunit_main",
        "//xls/common/fuzzing:fuzztest",
        "//xls/common/status:ret_check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "proc_id",
    srcs = ["proc_id.cc"],
    hdrs = ["proc_id.h"],
    deps = [
        ":ast",
        ":proc",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "proc_test_utils",
    testonly = True,
    srcs = ["proc_test_utils.cc"],
    hdrs = ["proc_test_utils.h"],
    deps = [
        ":ast",
        ":bindings",
        ":module",
        ":parser",
        ":pos",
        ":proc",
        ":scanner",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "proc_id_test",
    srcs = ["proc_id_test.cc"],
    deps = [
        ":ast",
        ":module",
        ":pos",
        ":proc",
        ":proc_id",
        ":proc_test_utils",
        "//xls/common:xls_gunit_main",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "zip_ast",
    srcs = ["zip_ast.cc"],
    hdrs = ["zip_ast.h"],
    deps = [
        ":ast",
        ":ast_node_visitor_with_default",
        "//xls/common/status:status_macros",
        "@com_google_absl//absl/functional:any_invocable",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "zip_ast_test",
    srcs = ["zip_ast_test.cc"],
    deps = [
        ":ast",
        ":ast_node_visitor_with_default",
        ":module",
        ":parser",
        ":pos",
        ":scanner",
        ":zip_ast",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "builtin_stubs_utils",
    srcs = ["builtin_stubs_utils.cc"],
    hdrs = [
        "builtin_stubs.h",
        "builtin_stubs_utils.h",
    ],
    deps = [
        ":ast",
        ":module",
        ":parser",
        ":pos",
        ":scanner",
        "@com_google_absl//absl/status:statusor",
    ],
)

genrule(
    name = "builtin_stubs_gen",
    srcs = ["builtin_stubs.x"],
    outs = ["builtin_stubs.h"],
    cmd = """(
      echo "#pragma once"
      echo "#include <string_view>"
      echo 'static inline constexpr std::string_view kBuiltinStubs = R"stubs('
      cat $<
      echo ')stubs";'
    ) > $@""",
)
