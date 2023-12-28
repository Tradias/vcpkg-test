cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(ignition-test VERSION 5.11.0)

# 
# ignition-msgs5:
# 
# find_package(ignition-cmake2 2.13 REQUIRED)
# find_package(Protobuf REQUIRED)

# ign_configure_project(VERSION_SUFFIX)

# message(STATUS "\n\n-- ====== Finding Dependencies ======")

# ign_find_package(IgnProtobuf
#                  REQUIRED
#                  COMPONENTS all
#                  PRETTY Protobuf)



# 
# ignition-msgs1:
# 
find_package(ignition-cmake0 0.3.0 REQUIRED)

ign_configure_project()

message(STATUS "\n\n-- ====== Finding Dependencies ======")

ign_find_package(IgnProtobuf
                 VERSION 3
                 PKGCONFIG protobuf
                 PKGCONFIG_VER_COMPARISON >=
                 REQUIRED
                 PRETTY Protobuf
                 PURPOSE ${PROTOBUF_PURPOSE})

#  custom:
ign_add_executable(ign_msgs_gen Generator.cc generator_main.cc)
target_link_libraries(ign_msgs_gen
  protobuf::libprotoc
  protobuf::libprotobuf)
target_include_directories(ign_msgs_gen PRIVATE ${PROTOBUF_INCLUDE_DIR})
target_compile_features(ign_msgs_gen PRIVATE ${IGN_CXX_11_FEATURES})

if (UNIX)
  target_link_libraries(ign_msgs_gen pthread)
endif()