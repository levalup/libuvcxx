cmake_minimum_required(VERSION 3.9)
project(libuvcxx)

# default using std=c++17 as libuvcxx has compatibility to C++11
set(STD "17" CACHE STRING "Set CMAKE_CXX_STANDARD")

option(WALL "If enable all compiler warning check" ON)
option(WERROR "If treat all warning as error" OFF)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")

# Load libuv from submodule or system
if (EXISTS "${PROJECT_SOURCE_DIR}/libuv/CMakeLists.txt")
    # compile static libuv
    option(LIBUV_BUILD_SHARED "" OFF)
    option(LIBUV_BUILD_TESTS "" OFF)
    option(LIBUV_BUILD_BENCH "" OFF)

    # for libuv <= 1.32.0
    option(BUILD_TESTING "" OFF)
    option(libuv_buildtests "" OFF)

    # compile libuv
    add_subdirectory(libuv)

    # using libuv
    include_directories("${PROJECT_SOURCE_DIR}/libuv/include")
    link_libraries(uv_a)

    message(STATUS "Using submodule libuv")
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

    message(STATUS "Using system libuv")
endif ()

include_directories("${PROJECT_SOURCE_DIR}/include")
file(GLOB_RECURSE HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")

if (STD)
    message(STATUS "Using --std=c++${STD}")
    set(CMAKE_CXX_STANDARD ${STD})
endif ()

if (WALL)
    message(STATUS "Enabled -Wall")
endif ()

if (WERROR)
    message(STATUS "Enabled -Werror")
endif ()

# build examples
add_subdirectory(examples)

# add tests
enable_testing()
add_subdirectory(tests)
