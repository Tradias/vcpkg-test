find_package(Shogun CONFIG REQUIRED)
find_package(Protobuf CONFIG)
find_package(spdlog CONFIG)

add_executable(test main.cpp)

target_link_libraries(test PRIVATE shogun::shogun-static protobuf::libprotobuf spdlog::spdlog)

target_compile_definitions(test PRIVATE SPDLOG_FMT_EXTERNAL)