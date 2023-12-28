cmake_minimum_required(VERSION 3.23)

project(test-vcpkg)

set(PROTO_DIR "${CMAKE_CURRENT_LIST_DIR}/proto")

add_subdirectory(src/asio-grpc)
# add_subdirectory(src/opentelemetry)
# add_subdirectory(src/gz)

# vcpkg commands: vcpkg format-manifest --all || vcpkg x-add-version --all
# https://github.com/microsoft/vcpkg/blob/master/docs/maintainers/maintainer-guide.md

# {
#     "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
#     "name": "test",
#     "version-string": "0.1.0",
#     "dependencies": [
#         "pugixml"
#     ],
#     "builtin-baseline": "8ea52bad1ac13e1afb892ccba6c261d8e82d06fd",
#     "overrides": [
#         {
#             "name": "pugixml",
#             "version": "1.11.4"
#         }
#     ]
# }