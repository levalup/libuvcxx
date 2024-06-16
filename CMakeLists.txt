cmake_minimum_required(VERSION 3.9)
project(libuvcxx)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# compile static libuv
option(LIBUV_BUILD_SHARED "" OFF)
option(LIBUV_BUILD_TESTS "" OFF)
option(LIBUV_BUILD_BENCH "" OFF)
add_subdirectory(libuv)

# using libuv
include_directories(libuv/include include)
link_libraries(libuv::libuv)

file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

# build examples
add_subdirectory(examples)

# add tests
enable_testing()
add_subdirectory(tests)
