find_package(libtorch CONFIG REQUIRED)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER "echo.proto")

add_executable(main ${PROTO_SRC} main.cpp)

target_link_libraries(main PRIVATE libtorch::libtorch)
target_include_directories(main PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
