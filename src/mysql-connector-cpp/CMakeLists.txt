find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)

include_directories(example ${CMAKE_CURRENT_BINARY_DIR})

add_executable(example example.cpp)
target_include_directories(example PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(example PRIVATE unofficial::mysql-connector-cpp::connector)
