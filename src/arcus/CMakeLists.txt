find_package(protobuf CONFIG REQUIRED)
find_package(Arcus CONFIG REQUIRED)

include_directories(example ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(example_PB_SRCS example_PB_HDRS "example.proto")

add_executable(example example.cpp ${example_PB_SRCS})
target_include_directories(example PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(example PRIVATE Arcus protobuf::libprotobuf)
