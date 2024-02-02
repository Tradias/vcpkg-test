cmake_minimum_required(VERSION 3.15...3.27)

project(main LANGUAGES CXX)

find_package(TBB CONFIG REQUIRED)

# cadexchanger uses CMAKE_COMPILER_IS_GNUCC which only looks at the C compiler not C++
enable_language(C)
find_package(
    CadEx
    COMPONENTS Core
    REQUIRED)

# lib
include(GenerateExportHeader)
add_library(lib SHARED)
generate_export_header(lib PREFIX_NAME THREEYD_ EXPORT_FILE_NAME generated/lib_export.h)
target_sources(lib PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated/lib_export.h" lib.cpp)
target_compile_definitions(lib PRIVATE lib_EXPORTS)
target_include_directories(lib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>)
set_target_properties(
    lib
    PROPERTIES C_VISIBILITY_PRESET "hidden"
               CXX_VISIBILITY_PRESET "hidden"
               VISIBILITY_INLINES_HIDDEN on)
target_link_libraries(lib PRIVATE TBB::tbb)

# main
add_executable(main)
target_sources(main PRIVATE main.cpp)
target_link_libraries(main PRIVATE CadEx::Core lib TBB::tbb)
