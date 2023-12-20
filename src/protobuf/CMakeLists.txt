cmake_minimum_required(VERSION 3.15...3.23)

project(test-protobuf)

find_package(protobuf CONFIG)

add_executable(test-protobuf main.cpp)
target_link_libraries(test-protobuf PRIVATE protobuf::libprotobuf)

# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/helloworld.proto)
protobuf_generate(
    TARGET
    test-protobuf
    IMPORT_DIRS
    "${PROTO_DIR}"
    PROTOS
    "${PROTO_DIR}/helloWorld.proto"
    LANGUAGE
    cpp
    PROTOC_OUT_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_include_directories(test-protobuf PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src"
                                                 "${CMAKE_CURRENT_BINARY_DIR}/generated")
