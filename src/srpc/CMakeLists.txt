find_package(srpc CONFIG REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_package(Snappy CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(ZLIB)
find_library(srpc srpc)
find_library(workflow workflow)

include_directories(example ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(example_PB_SRCS example_PB_HDRS "example.proto")

add_executable(example example.cpp ${example_PB_SRCS})
target_include_directories(example PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(example PRIVATE protobuf::libprotobuf ${srpc} ${workflow} lz4::lz4 Snappy::snappy ZLIB::ZLIB)
