cmake_minimum_required(VERSION 3.15...3.23)

project(test-upb)

find_package(protobuf CONFIG)
find_package(upb CONFIG REQUIRED)
# find_package(utf8_range CONFIG REQUIRED)

set(UPB_DESCRIPTOR_SOURCES
    "${CMAKE_CURRENT_BINARY_DIR}/generated/helloWorld.upb.h"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/helloWorld.upb.c"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/helloWorld.upbdefs.h"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/helloWorld.upbdefs.c"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/helloWorld.upb_minitable.h"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/helloWorld.upb_minitable.c")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
add_custom_command(
    OUTPUT ${UPB_DESCRIPTOR_SOURCES}
    DEPENDS "${PROTO_DIR}/helloWorld.proto"
    COMMAND
        protobuf::protoc "-I${PROTO_DIR}/"
        "--plugin=protoc-gen-upb=$<TARGET_FILE:upb::protoc-gen-upb>"
        "--plugin=protoc-gen-upbdefs=$<TARGET_FILE:upb::protoc-gen-upbdefs>"
        "--plugin=protoc-gen-upb_minitable=$<TARGET_FILE:upb::protoc-gen-upb_minitable>"
        "--upb_out=${CMAKE_CURRENT_BINARY_DIR}/generated" "--upbdefs_out=${CMAKE_CURRENT_BINARY_DIR}/generated"
        "--upb_minitable_out=${CMAKE_CURRENT_BINARY_DIR}/generated"
        "${PROTO_DIR}/helloWorld.proto")

add_executable(test-upb)
target_sources(test-upb PRIVATE main.cpp ${UPB_DESCRIPTOR_SOURCES})
target_include_directories(test-upb PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(test-upb PRIVATE upb::upb)

# upb_base upb_mem upb_hash upb_tokenizer upb_chunked_stream upb_json upb_lex upb_accessors upb_copy
# upb_message_internal upb_promote upb_mini_descriptor upb_mini_descriptor_internal upb_compat upb_mini_table
# upb_mini_table_internal upb_reflection_internal upb_text upb_wire_internal utf8_range upb_reader
# upb_descriptor_upb_proto

# target_compile_features(mod1 PUBLIC cxx_std_23) target_compile_features(test PRIVATE cxx_std_20)

# target_precompile_headers(test PRIVATE "src/precompiledHeader.h")

# target_compile_options(test PRIVATE -ftime-trace) target_compile_options(test PRIVATE /Bt+)

# vcpkg commands: vcpkg format-manifest --all || vcpkg x-add-version --all
# https://github.com/microsoft/vcpkg/blob/master/docs/maintainers/maintainer-guide.md
