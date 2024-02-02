find_package(unofficial-brpc CONFIG REQUIRED)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER "echo.proto")

add_executable(server ${PROTO_SRC} server.cpp)
add_executable(client ${PROTO_SRC} client.cpp)

foreach(_target client server)
    target_link_libraries(${_target} PRIVATE unofficial::brpc::brpc-static)
    target_include_directories(${_target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
endforeach()
