# Copyright 2020 The XLS Authors
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

# Domain Specific Language implementation for HW construction that mimics Rust.

load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

# pytype tests are present in this file
load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")
load("@xls_pip_deps//:requirements.bzl", "requirement")

package(
    default_applicable_licenses = ["//:license"],
    default_visibility = ["//xls:xls_internal"],
    features = [
        "layering_check",
        "parse_headers",
    ],
    licenses = ["notice"],  # Apache 2.0
)

bzl_library(
    name = "strip_comments_bzl",
    srcs = ["strip_comments.bzl"],
)

cc_library(
    name = "virtualizable_file_system",
    srcs = ["virtualizable_file_system.cc"],
    hdrs = ["virtualizable_file_system.h"],
    deps = [
        "//xls/common/file:filesystem",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "constexpr_evaluator",
    srcs = ["constexpr_evaluator.cc"],
    hdrs = ["constexpr_evaluator.h"],
    deps = [
        ":errors",
        ":import_data",
        ":interp_value",
        ":interp_value_utils",
        ":warning_collector",
        ":warning_kind",
        "//xls/common:visitor",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx/bytecode",
        "//xls/dslx/bytecode:bytecode_emitter",
        "//xls/dslx/bytecode:bytecode_interpreter",
        "//xls/dslx/bytecode:bytecode_interpreter_options",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:ast_utils",
        "//xls/dslx/frontend:module",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/type_system:deduce_utils",
        "//xls/dslx/type_system:parametric_env",
        "//xls/dslx/type_system:type",
        "//xls/dslx/type_system:type_info",
        "//xls/dslx/type_system:type_zero_value",
        "//xls/dslx/type_system:unwrap_meta_type",
        "//xls/ir:bits",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "constexpr_evaluator_test",
    srcs = ["constexpr_evaluator_test.cc"],
    deps = [
        ":constexpr_evaluator",
        ":create_import_data",
        ":import_data",
        ":interp_value",
        ":parse_and_typecheck",
        ":warning_collector",
        ":warning_kind",
        "//xls/common:casts",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:parser",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/frontend:scanner",
        "//xls/dslx/type_system:parametric_env",
        "//xls/dslx/type_system:type",
        "//xls/dslx/type_system:type_info",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_absl//absl/status:statusor",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "create_import_data",
    srcs = ["create_import_data.cc"],
    hdrs = ["create_import_data.h"],
    deps = [
        ":default_dslx_stdlib_path",
        ":import_data",
        ":virtualizable_file_system",
        ":warning_kind",
        "//xls/dslx/bytecode:bytecode_cache",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "errors",
    srcs = ["errors.cc"],
    hdrs = ["errors.h"],
    deps = [
        ":import_record",
        ":interp_value",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:ast_node",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/type_system:type",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "errors_test",
    srcs = ["errors_test.cc"],
    deps = [
        ":errors",
        "//xls/common:xls_gunit_main",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/type_system:type",
        "@com_google_absl//absl/status",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "interp_value_utils",
    srcs = ["interp_value_utils.cc"],
    hdrs = ["interp_value_utils.h"],
    deps = [
        ":channel_direction",
        ":interp_value",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/type_system:type",
        "//xls/ir:bits",
        "//xls/ir:bits_ops",
        "//xls/ir:ir_parser",
        "//xls/ir:value",
        "@com_google_absl//absl/functional:function_ref",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "interp_value_utils_test",
    srcs = ["interp_value_utils_test.cc"],
    deps = [
        ":interp_value",
        ":interp_value_utils",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:module",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/type_system:type",
        "//xls/ir:bits",
        "//xls/ir:value",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_absl//absl/strings",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "trait_visitor",
    hdrs = ["trait_visitor.h"],
    deps = [
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:ast_node_visitor_with_default",
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "value_format_descriptor",
    srcs = ["value_format_descriptor.cc"],
    hdrs = ["value_format_descriptor.h"],
    deps = [
        "//xls/ir:bits",
        "//xls/ir:format_preference",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "interp_value",
    srcs = ["interp_value.cc"],
    hdrs = ["interp_value.h"],
    deps = [
        ":channel_direction",
        ":dslx_builtins",
        ":value_format_descriptor",
        "//xls/common:math_util",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/data_structures:inline_bitmap",
        "//xls/dslx/frontend:ast",
        "//xls/ir:bits",
        "//xls/ir:bits_ops",
        "//xls/ir:format_preference",
        "//xls/ir:value",
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
    name = "interp_value_test",
    srcs = ["interp_value_test.cc"],
    deps = [
        ":interp_value",
        ":parse_and_typecheck",
        ":value_format_descriptor",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:pos",
        "//xls/ir:bits",
        "//xls/ir:format_preference",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_absl//absl/status:statusor",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "interp_bindings",
    srcs = ["interp_bindings.cc"],
    hdrs = ["interp_bindings.h"],
    deps = [
        ":interp_value",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/type_system:parametric_env",
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
    name = "interp_bindings_test",
    srcs = ["interp_bindings_test.cc"],
    deps = [
        ":interp_bindings",
        ":interp_value",
        "//xls/common:xls_gunit_main",
        "//xls/dslx/frontend:pos",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_absl//absl/status:statusor",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "import_record",
    hdrs = ["import_record.h"],
    deps = [
        "//xls/dslx/frontend:pos",
    ],
)

cc_library(
    name = "import_data",
    srcs = ["import_data.cc"],
    hdrs = ["import_data.h"],
    deps = [
        ":errors",
        ":import_record",
        ":interp_bindings",
        ":virtualizable_file_system",
        ":warning_kind",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx/bytecode:bytecode_cache_interface",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:ast_node",
        "//xls/dslx/frontend:module",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/type_system:type_info",
        "//xls/dslx/type_system_v2:inference_table",
        "//xls/dslx/type_system_v2:inference_table_converter",
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

cc_library(
    name = "import_routines",
    srcs = ["import_routines.cc"],
    hdrs = [
        "import_routines.h",
        ":stdlib_builtins.inc",
    ],
    data = ["//xls/dslx/stdlib:x_files"],
    deps = [
        ":import_data",
        ":virtualizable_file_system",
        "//xls/common/config:xls_config",
        "//xls/common/file:get_runfile_path",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:module",
        "//xls/dslx/frontend:parser",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/frontend:scanner",
        "@com_google_absl//absl/cleanup",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "mangle",
    srcs = ["mangle.cc"],
    hdrs = ["mangle.h"],
    deps = [
        ":interp_value",
        "//xls/dslx/type_system:parametric_env",
        "//xls/ir:format_preference",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "mangle_test",
    srcs = ["mangle_test.cc"],
    deps = [
        ":interp_value",
        ":mangle",
        "//xls/common:xls_gunit_main",
        "//xls/dslx/type_system:parametric_env",
        "@com_google_absl//absl/status:status_matchers",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "dslx_builtins",
    srcs = ["dslx_builtins.cc"],
    hdrs = ["dslx_builtins.h"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "dslx_builtins_signatures",
    srcs = ["dslx_builtins_signatures.cc"],
    hdrs = ["dslx_builtins_signatures.h"],
    deps = [
        ":channel_direction",
        ":errors",
        ":interp_value",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:ast_node",
        "//xls/dslx/frontend:builtins_metadata",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/type_system:deduce_ctx",
        "//xls/dslx/type_system:parametric_instantiator",
        "//xls/dslx/type_system:parametric_with_type",
        "//xls/dslx/type_system:type",
        "//xls/dslx/type_system:type_and_parametric_env",
        "//xls/dslx/type_system:unwrap_meta_type",
        "@com_google_absl//absl/base:no_destructor",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "error_printer",
    srcs = ["error_printer.cc"],
    hdrs = ["error_printer.h"],
    deps = [
        ":virtualizable_file_system",
        ":warning_collector",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:pos",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@re2",
    ],
)

cc_test(
    name = "error_printer_test",
    srcs = ["error_printer_test.cc"],
    deps = [
        ":error_printer",
        ":virtualizable_file_system",
        "//xls/common:xls_gunit_main",
        "//xls/common/file:temp_file",
        "//xls/common/status:matchers",
        "//xls/dslx/frontend:pos",
        "@com_google_absl//absl/strings",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "parse_and_typecheck",
    srcs = ["parse_and_typecheck.cc"],
    hdrs = ["parse_and_typecheck.h"],
    deps = [
        ":import_data",
        ":warning_collector",
        ":warning_kind",
        "//xls/common/file:get_runfile_path",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:comment_data",
        "//xls/dslx/frontend:module",
        "//xls/dslx/frontend:parser",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/frontend:scanner",
        "//xls/dslx/type_system:type_info",
        "//xls/dslx/type_system:typecheck_module",
        "//xls/dslx/type_system_v2:typecheck_module_v2",
        "@com_google_absl//absl/cleanup",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "default_dslx_stdlib_path",
    srcs = ["default_dslx_stdlib_path.inc"],
    hdrs = ["default_dslx_stdlib_path.h"],
)

cc_library(
    name = "command_line_utils",
    srcs = ["command_line_utils.cc"],
    hdrs = ["command_line_utils.h"],
    deps = [
        ":error_printer",
        ":virtualizable_file_system",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:bindings",
        "//xls/dslx/frontend:pos",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "command_line_utils_test",
    srcs = ["command_line_utils_test.cc"],
    deps = [
        ":command_line_utils",
        "//xls/common:xls_gunit_main",
        "@com_google_absl//absl/status:status_matchers",
        "@googletest//:gtest",
    ],
)

#  -- DSLX specific tool binaries / command line utilities

cc_binary(
    name = "interpreter_main",
    srcs = ["interpreter_main.cc"],
    visibility = ["//xls:xls_users"],
    deps = [
        ":command_line_utils",
        ":default_dslx_stdlib_path",
        ":parse_and_typecheck",
        ":virtualizable_file_system",
        ":warning_kind",
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/file:filesystem",
        "//xls/common/status:status_macros",
        "//xls/dslx/run_routines",
        "//xls/dslx/run_routines:ir_test_runner",
        "//xls/dslx/run_routines:run_comparator",
        "//xls/dslx/run_routines:test_xml",
        "//xls/ir:format_preference",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
        "@re2",
    ],
)

cc_binary(
    name = "parse_and_typecheck_dslx_main",
    srcs = ["parse_and_typecheck_dslx_main.cc"],
    deps = [
        ":command_line_utils",
        ":create_import_data",
        ":default_dslx_stdlib_path",
        ":parse_and_typecheck",
        ":virtualizable_file_system",
        ":warning_kind",
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/status:status_macros",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_binary(
    name = "prove_quickcheck_main",
    srcs = ["prove_quickcheck_main.cc"],
    deps = [
        ":command_line_utils",
        ":default_dslx_stdlib_path",
        ":parse_and_typecheck",
        ":warning_kind",
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/file:filesystem",
        "//xls/common/status:status_macros",
        "//xls/dslx/run_routines",
        "//xls/dslx/run_routines:test_xml",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
        "@re2",
    ],
)

cc_binary(
    name = "highlight_main",
    srcs = ["highlight_main.cc"],
    deps = [
        ":virtualizable_file_system",
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:builtins_metadata",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/frontend:scanner",
        "//xls/dslx/frontend:token",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

py_test(
    name = "highlight_main_test",
    srcs = ["highlight_main_test.py"],
    data = [
        ":highlight_main",
        "//xls/examples:sha256.x",
    ],
    deps = [
        "//xls/common:runfiles",
        "@abseil-py//absl/testing:absltest",
    ],
)

cc_binary(
    name = "strip_comments_main",
    srcs = ["strip_comments_main.cc"],
    deps = [
        ":virtualizable_file_system",
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/frontend:scanner",
        "//xls/dslx/frontend:token",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

py_test(
    name = "strip_comments_main_test",
    srcs = ["strip_comments_main_test.py"],
    data = [
        ":strip_comments_main",
        "//xls/examples:sha256.x",
    ],
    deps = [
        "//xls/common:runfiles",
        "@abseil-py//absl/testing:absltest",
    ],
)

py_test(
    name = "interpreter_test",
    srcs = ["interpreter_test.py"],
    data = [":interpreter_main"],
    deps = [
        "//xls/common:runfiles",
        "//xls/common:test_base",
    ],
)

py_test(
    name = "prove_quickcheck_test",
    srcs = ["prove_quickcheck_test.py"],
    data = [":prove_quickcheck_main"],
    deps = [
        "//xls/common:runfiles",
        "//xls/common:test_base",
    ],
)

cc_library(
    name = "warning_kind",
    srcs = ["warning_kind.cc"],
    hdrs = ["warning_kind.h"],
    deps = [
        "//xls/common:strong_int",
        "//xls/common/status:status_macros",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "warning_kind_test",
    srcs = ["warning_kind_test.cc"],
    deps = [
        ":warning_kind",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_absl//absl/status:statusor",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "warning_collector",
    hdrs = ["warning_collector.h"],
    deps = [
        ":warning_kind",
        "//xls/dslx/frontend:pos",
    ],
)

cc_library(
    name = "make_value_format_descriptor",
    srcs = ["make_value_format_descriptor.cc"],
    hdrs = ["make_value_format_descriptor.h"],
    deps = [
        ":interp_value",
        ":value_format_descriptor",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/type_system:type",
        "//xls/ir:bits",
        "//xls/ir:format_preference",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "channel_direction",
    hdrs = ["channel_direction.h"],
)

cc_library(
    name = "extract_module_name",
    srcs = ["extract_module_name.cc"],
    hdrs = ["extract_module_name.h"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "shadow_builtin_test",
    srcs = ["shadow_builtin_test.cc"],
    deps = [
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/dslx/frontend:builtins_metadata",
        "//xls/dslx/run_routines",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "interp_value_from_string",
    srcs = ["interp_value_from_string.cc"],
    hdrs = ["interp_value_from_string.h"],
    data = ["//xls/dslx/stdlib:x_files"],
    deps = [
        ":create_import_data",
        ":import_data",
        ":interp_value",
        ":parse_and_typecheck",
        ":virtualizable_file_system",
        ":warning_kind",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:ast",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "interp_value_from_string_test",
    srcs = ["interp_value_from_string_test.cc"],
    deps = [
        ":default_dslx_stdlib_path",
        ":interp_value",
        ":interp_value_from_string",
        "//xls/common:xls_gunit_main",
        "@com_google_absl//absl/status:status_matchers",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "error_test_utils",
    testonly = True,
    hdrs = ["error_test_utils.h"],
    deps = [
        "//xls/dslx/frontend:bindings",
        "//xls/dslx/frontend:pos",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@googletest//:gtest",
    ],
)

cc_test(
    name = "error_test_utils_test",
    srcs = ["error_test_utils_test.cc"],
    deps = [
        ":error_test_utils",
        "//xls/common:xls_gunit_main",
        "//xls/dslx/frontend:pos",
        "//xls/dslx/frontend:scanner",
        "@com_google_absl//absl/status",
        "@googletest//:gtest",
    ],
)

cc_binary(
    name = "dslx_fmt",
    srcs = ["dslx_fmt.cc"],
    deps = [
        ":command_line_utils",
        ":create_import_data",
        ":default_dslx_stdlib_path",
        ":import_data",
        ":parse_and_typecheck",
        ":virtualizable_file_system",
        ":warning_kind",
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/file:filesystem",
        "//xls/common/status:status_macros",
        "//xls/dslx/fmt:ast_fmt",
        "//xls/dslx/fmt:comments",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:comment_data",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

py_test(
    name = "dslx_fmt_test",
    srcs = ["dslx_fmt_test.py"],
    data = [
        ":dslx_fmt",
        "//xls/examples:sha256.x",
    ],
    deps = [
        "//xls/common:runfiles",
        "@abseil-py//absl/testing:absltest",
    ],
)

# -- Generated stdlib built-ins

cc_binary(
    name = "generate_stdlib_builtins_main",
    srcs = ["generate_stdlib_builtins_main.cc"],
    deps = [
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

genrule(
    name = "stdlib_builtins_inc",
    srcs = ["//xls/dslx/stdlib:x_files"],
    outs = ["stdlib_builtins.inc"],
    cmd = "$(location :generate_stdlib_builtins_main) $(location stdlib_builtins.inc) $(SRCS)",
    tools = [":generate_stdlib_builtins_main"],
)
