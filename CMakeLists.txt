cmake_minimum_required(VERSION 3.9)
project(libuvcxx)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if (EXISTS "libuv/CMakeLists.txt")
    # compile static libuv
    option(LIBUV_BUILD_SHARED "" OFF)
    option(LIBUV_BUILD_TESTS "" OFF)
    option(LIBUV_BUILD_BENCH "" OFF)
    add_subdirectory(libuv)

    # using libuv
    include_directories(libuv/include)
    link_libraries(libuv::libuv)
else ()
    # find libuv in system
    find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
    find_library(LIBUV_LIBRARY NAMES uv libuv)

    if (LIBUV_LIBRARY AND LIBUV_INCLUDE_DIR)
        # using libuv
        include_directories(${LIBUV_INCLUDE_DIR})
        link_libraries(${LIBUV_LIBRARY})
    else ()
        # find libuv witch pkg-config
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)

        # using libuv
        include_directories(${LIBUV_INCLUDE_DIRS})
        link_libraries(PkgConfig::LIBUV)
    endif ()
endif ()

include_directories(include)

file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

# build examples
add_subdirectory(examples)

# add tests
enable_testing()
add_subdirectory(tests)
