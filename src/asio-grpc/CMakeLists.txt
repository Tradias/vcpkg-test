cmake_minimum_required(VERSION 3.23)

project(test-asio-grpc)

# find_package(protobuf CONFIG) 
# find_package(Boost CONFIG) 
# find_package(gRPC CONFIG REQUIRED)
# include(FindPkgConfig)
# pkg_search_module(gRPC REQUIRED IMPORTED_TARGET grpc++)
# set(ASIO_GRPC_DISABLE_AUTOLINK on)
# find_package(asio CONFIG REQUIRED)
find_package(asio-grpc CONFIG REQUIRED)

# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/helloworld.proto) target_sources(test PRIVATE main.cpp
# dataProvider.cpp)

# add_library(coroutines INTERFACE)
# add_library(std::coroutines ALIAS coroutines)

add_executable(test-asio-grpc main.cpp) 
target_link_libraries(test-asio-grpc PRIVATE asio-grpc::asio-grpc-standalone-asio)

asio_grpc_protobuf_generate(
    GENERATE_GRPC
    TARGET test-asio-grpc
    OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
    PROTOS "${PROTO_DIR}/helloWorld.proto" "${PROTO_DIR}/messages.proto")

# protobuf_generate(
#     TARGET test-asio-grpc
#     IMPORT_DIRS "${PROTO_DIR}"
#     PROTOS "${PROTO_DIR}/helloWorld.proto"
#     LANGUAGE cpp
#     PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
#   )

# protobuf_generate(  
#     TARGET test-asio-grpc
#     IMPORT_DIRS "${PROTO_DIR}"
#     PROTOS "${PROTO_DIR}/helloWorld.proto"
#     LANGUAGE grpc
#     GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
#     PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
#     PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
#   )
# file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_include_directories(test-asio-grpc PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")

# target_compile_definitions(
#     test-asio-grpc
#     PRIVATE WIN32_LEAN_AND_MEAN
#             BOOST_ASIO_NO_DEPRECATED
#             ASIO_NO_DEPRECATED)

# target_compile_options(
#     test-asio-grpc
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

# target_compile_features(test-asio-grpc PRIVATE cxx_std_20)

# target_precompile_headers(test-asio-grpc PRIVATE "precompiledHeader.h")

# target_compile_options(test-asio-grpc PRIVATE -ftime-trace) target_compile_options(test-asio-grpc PRIVATE /Bt+)

# vcpkg commands: vcpkg format-manifest --all || vcpkg x-add-version --all
# https://github.com/microsoft/vcpkg/blob/master/docs/maintainers/maintainer-guide.md
