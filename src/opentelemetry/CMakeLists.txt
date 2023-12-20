cmake_minimum_required(VERSION 3.15...3.23)

project(test-opentelemetry)

# find_package(protobuf CONFIG) 
# find_package(Boost CONFIG) 
# find_package(gRPC CONFIG)
# find_package(asio CONFIG REQUIRED)
find_package(asio-grpc CONFIG REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)

add_executable(test-opentelemetry main.cpp) 
target_link_libraries(test-opentelemetry PRIVATE asio-grpc::asio-grpc 
opentelemetry-cpp::ostream_span_exporter 
opentelemetry-cpp::otlp_grpc_exporter)

protobuf_generate(
    TARGET test-opentelemetry
    IMPORT_DIRS "${PROTO_DIR}"
    PROTOS "${PROTO_DIR}/messages.proto"
    LANGUAGE cpp
    PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
  )

protobuf_generate(  
    TARGET test-opentelemetry
    IMPORT_DIRS "${PROTO_DIR}"
    PROTOS "${PROTO_DIR}/messages.proto"
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
  )
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_include_directories(test-opentelemetry PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/generated")

# target_compile_definitions(
#     test
#     PRIVATE WIN32_LEAN_AND_MEAN
#             BOOST_ASIO_NO_DEPRECATED
#             ASIO_NO_DEPRECATED)

# target_compile_options(
#     test
#     PUBLIC 
#                   $<$<OR:$<CXX_COMPILER_ID:MSVC>,$<STREQUAL:${CMAKE_CXX_COMPILER_FRONTEND_VARIANT},MSVC>>:
#                   /W4
#                   /wd4996
#                   /permissive-
#                   /Zc:__cplusplus
#                   /Zc:inline
#                   /Zc:sizedDealloc>
#                   $<$<CXX_COMPILER_ID:MSVC>:
#                   /Zc:preprocessor
#                   /Zc:externConstexpr
#                   /Zc:lambda
#                   /Zc:throwingNew>
#                   $<$<OR:$<CXX_COMPILER_ID:GNU,AppleClang>,$<STREQUAL:${CMAKE_CXX_COMPILER_FRONTEND_VARIANT},GNU>>:
#                   -Wall
#                   -Wextra
#                   -Wno-deprecated-declarations
#                   -pedantic-errors>
#                   $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-self-move>)

# target_compile_features(mod1 PUBLIC cxx_std_23)
target_compile_features(test-opentelemetry PRIVATE cxx_std_20)

target_precompile_headers(test-opentelemetry PRIVATE "precompiledHeader.h")

# target_compile_options(test PRIVATE -ftime-trace) target_compile_options(test PRIVATE /Bt+)

# vcpkg commands: vcpkg format-manifest --all || vcpkg x-add-version --all
# https://github.com/microsoft/vcpkg/blob/master/docs/maintainers/maintainer-guide.md
