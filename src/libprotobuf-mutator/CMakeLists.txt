find_package(libprotobuf-mutator CONFIG REQUIRED)

# protobuf_generate_cpp(PROTO_SRC PROTO_HEADER "echo.proto")
  
add_executable(main ${PROTO_SRC} main.cpp)

target_link_libraries(main PRIVATE libprotobuf-mutator::protobuf-mutator protobuf::libprotobuf)
target_include_directories(main PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

protobuf_generate(
    TARGET main
    IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}"
    PROTOS "echo.proto"
    LANGUAGE cpp
    PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
)