#
# Copyright 2018 Asylo authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

load("//asylo/bazel:asylo.bzl", "cc_test", "embed_enclaves", "sim_enclave_test")
load("//asylo/bazel:copts.bzl", "ASYLO_DEFAULT_COPTS")

licenses(["notice"])  # Apache v2.0

# Common utilities for trusted and untrusted targets.

# Defines ASYLO macros.
cc_library(
    name = "asylo_macros",
    hdrs = ["asylo_macros.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
)

# Implementation of binary search over size_t.
cc_library(
    name = "binary_search",
    hdrs = ["binary_search.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
)

cc_test(
    name = "binary_search_test",
    srcs = ["binary_search_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    enclave_test_name = "binary_search_enclave_test",
    deps = [
        ":binary_search",
        "//asylo/test/util:test_main",
        "@com_google_googletest//:gtest",
    ],
)

# A library for cleanup objects.
cc_library(
    name = "cleanup",
    hdrs = ["cleanup.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
)

cc_test(
    name = "cleanup_test",
    srcs = ["cleanup_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    enclave_test_name = "cleanup_enclave_test",
    deps = [
        ":cleanup",
        "//asylo/test/util:test_main",
        "@com_google_googletest//:gtest",
    ],
)

# Utility for reading ELF files.
cc_library(
    name = "elf_reader",
    srcs = ["elf_reader.cc"],
    hdrs = ["elf_reader.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
    deps = [
        ":status",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

# Embedded section to view in ElfReader unit test.
ELF_READER_TEST_SECTION = "foo_section"

embed_enclaves(
    name = "elf_reader_test_binary",
    elf_file = "//asylo/test/util:test_binary",
    enclaves = {ELF_READER_TEST_SECTION: "//asylo/test/util:sample_text"},
    executable = 1,
)

cc_test(
    name = "elf_reader_test",
    srcs = ["elf_reader_test.cc"],
    args = [
        "--elf_file",
        "$(location :elf_reader_test_binary)",
        "--section_name",
        "%s" % ELF_READER_TEST_SECTION,
        "--expected_contents",
        "$(location //asylo/test/util:sample_text)",
    ],
    copts = ASYLO_DEFAULT_COPTS,
    data = [
        ":elf_reader_test_binary",
        "//asylo/test/util:sample_text",
    ],
    deps = [
        ":elf_reader",
        ":file_mapping",
        "//asylo/test/util:status_matchers",
        "//asylo/test/util:test_main",
        "//asylo/util:logging",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest",
    ],
)

# A library of utilities for working with POSIX file descriptors.
cc_library(
    name = "fd_utils",
    srcs = ["fd_utils.cc"],
    hdrs = ["fd_utils.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
    deps = [
        ":status",
        "//asylo/util:logging",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

cc_test(
    name = "fd_utils_test",
    srcs = ["fd_utils_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    enclave_test_name = "fd_utils_enclave_test",
    deps = [
        ":fd_utils",
        "//asylo/test/util:status_matchers",
        "//asylo/test/util:test_main",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest",
    ],
)

# A library for memory-mapping files.
cc_library(
    name = "file_mapping",
    srcs = ["file_mapping.cc"],
    hdrs = ["file_mapping.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
    deps = [
        ":cleanup",
        ":status",
        "//asylo/util:logging",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "file_mapping_test",
    srcs = ["file_mapping_test.cc"],
    args = [
        "--test_file",
        "$(location //asylo/test/util:sample_text)",
    ],
    copts = ASYLO_DEFAULT_COPTS,
    data = ["//asylo/test/util:sample_text"],
    deps = [
        ":file_mapping",
        ":status",
        "//asylo/test/util:status_matchers",
        "//asylo/test/util:test_main",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest",
    ],
)

# Minimal logging library implementation.
cc_library(
    name = "logging",
    srcs = ["logging.cc"],
    hdrs = ["logging.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = ["@com_google_absl//absl/base:core_headers"],
)

cc_library(
    name = "mutex_guarded",
    hdrs = ["mutex_guarded.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "mutex_guarded_test",
    srcs = ["mutex_guarded_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    tags = ["noregression"],
    deps = [
        ":mutex_guarded",
        "//asylo/test/util:test_main",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "error_codes",
    hdrs = ["error_codes.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
)

cc_library(
    name = "status_macros",
    hdrs = ["status_macros.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
    ],
)

cc_library(
    name = "status",
    srcs = [
        "error_codes.h",
        "error_space.cc",
        "error_space.h",
        "posix_error_space.cc",
        "status.cc",
        "status_error_space.cc",
        "status_error_space.h",
        "status_internal.h",
    ],
    hdrs = [
        "posix_error_space.h",
        "status.h",
        "status_macros.h",
        "statusor.h",
    ],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":status_cc_proto",
        ":status_macros",
        "//asylo/platform/common:static_map",
        "//asylo/util:logging",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/strings",
    ],
)

# Tests for the Status utility.
cc_test(
    name = "status_test",
    srcs = ["status_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    enclave_test_name = "status_enclave_test",
    deps = [
        ":status",
        "//asylo/test/util:status_matchers",
        "//asylo/test/util:test_main",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_googletest//:gtest",
    ],
)

# Tests for the StatusOr template class.
cc_test(
    name = "statusor_test",
    srcs = [
        "error_codes.h",
        "error_space.h",
        "status_error_space.h",
        "statusor_test.cc",
    ],
    copts = ASYLO_DEFAULT_COPTS,
    enclave_test_name = "statusor_enclave_test",
    deps = [
        ":status",
        "//asylo/platform/common:static_map",
        "//asylo/test/util:status_matchers",
        "//asylo/test/util:test_main",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_googletest//:gtest",
    ],
)

# Tests for the Status macros.
cc_test(
    name = "status_macros_test",
    srcs = ["status_macros_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    deps = [
        ":status",
        "//asylo/test/util:status_matchers",
        "//asylo/test/util:test_main",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest",
    ],
)

# Container types that zero-out memory before freeing resources.
cc_library(
    name = "cleansing_types",
    hdrs = [
        "cleansing_allocator.h",
        "cleansing_types.h",
    ],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = ["@boringssl//:crypto"],
)

cc_test(
    name = "cleansing_allocator_test",
    srcs = [
        "cleansing_allocator.h",
        "cleansing_allocator_test.cc",
    ],
    copts = ASYLO_DEFAULT_COPTS,
    deps = [
        "//asylo/crypto/util:bytes",
        "//asylo/crypto/util:trivial_object_util",
        "//asylo/test/util:test_main",
        "@boringssl//:crypto",
        "@com_google_googletest//:gtest",
    ],
)

# Tests for Google canonical error space.
cc_test(
    name = "error_space_test",
    srcs = [
        "error_codes.h",
        "error_space.h",
        "error_space_test.cc",
    ],
    copts = ASYLO_DEFAULT_COPTS,
    deps = [
        ":status",
        "//asylo/platform/common:static_map",
        "//asylo/test/util:test_main",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_googletest//:gtest",
    ],
)

# Tests for Posix error space.
cc_test(
    name = "posix_error_space_test",
    srcs = ["posix_error_space_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    deps = [
        ":status",
        "//asylo/test/util:test_main",
        "@com_google_googletest//:gtest",
    ],
)

# A library to help with the google.protobuf.Value representation of JSON.
cc_library(
    name = "proto_struct_util",
    srcs = ["proto_struct_util.cc"],
    hdrs = ["proto_struct_util.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
    deps = [
        ":status",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:cc_wkt_protos",
    ],
)

# Tests for proto struct utilities.
cc_test(
    name = "proto_struct_util_test",
    srcs = ["proto_struct_util_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    enclave_test_name = "proto_struct_util_enclave_test",
    deps = [
        ":proto_struct_util",
        ":status",
        "//asylo/test/util:proto_matchers",
        "//asylo/test/util:status_matchers",
        "//asylo/test/util:test_main",
        "@com_google_googletest//:gtest",
        "@com_google_protobuf//:cc_wkt_protos",
    ],
)

# Protobuf representation for asylo::Status.
proto_library(
    name = "status_proto",
    srcs = ["status.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "status_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":status_proto"],
)

# Library for manipulating file paths.
cc_library(
    name = "path",
    srcs = ["path.cc"],
    hdrs = ["path.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = ["@com_google_absl//absl/strings"],
)

# Test for the path library.
cc_test(
    name = "path_test",
    srcs = ["path_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    enclave_test_name = "path_enclave_test",
    deps = [
        ":path",
        "//asylo/test/util:test_main",
        "@com_google_googletest//:gtest",
    ],
)

# A library that implements asylo::Thread
cc_library(
    name = "thread",
    srcs = ["std_thread.h"],
    hdrs = ["thread.h"],
    copts = ASYLO_DEFAULT_COPTS,
    visibility = ["//asylo:implementation"],
    deps =
        [
            "//asylo/util:logging",
        ],
)

cc_test(
    name = "thread_test",
    srcs = ["thread_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    enclave_test_name = "thread_enclave_test",
    deps = [
        ":mutex_guarded",
        ":thread",
        "//asylo/test/util:test_main",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest",
    ],
)

sim_enclave_test(
    name = "primitives_thread_test",
    srcs = ["thread_test.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    deps = [
        ":mutex_guarded",
        ":thread",
        "//asylo/test/util:test_main",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest",
    ],
)
