cmake_minimum_required(VERSION 3.9)
project(libuvcxx)

# -------------------------------------------------------------------------------
# Options
# -------------------------------------------------------------------------------

# default using std=c++17 as libuvcxx has compatibility to C++11
set(STD "17" CACHE STRING "Set CMAKE_CXX_STANDARD")

option(WALL "If enable all compiler warning check" ON)
option(WERROR "If treat all warning as error" OFF)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")

# -------------------------------------------------------------------------------
# Load libuv from submodule or system
# -------------------------------------------------------------------------------

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

# -------------------------------------------------------------------------------
# Message configuration items
# -------------------------------------------------------------------------------

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

# -------------------------------------------------------------------------------
# Collect header files
# -------------------------------------------------------------------------------

include_directories("${PROJECT_SOURCE_DIR}/include")
file(GLOB_RECURSE HEADERS "${PROJECT_SOURCE_DIR}/include/uvcxx/*.h")

# -------------------------------------------------------------------------------
# Generate single header
# -------------------------------------------------------------------------------

find_package(Python 3 COMPONENTS Interpreter)

set(ENTRY_HEADER "${PROJECT_SOURCE_DIR}/include/uvcxx.h")
set(SINGLE_HEADER "${PROJECT_SOURCE_DIR}/include/uvcxx-single.h")

if (Python_FOUND)
    if ("${Python_VERSION_MAJOR}" GREATER 3
            OR ("${Python_VERSION_MAJOR}" EQUAL 3 AND "${Python_VERSION_MINOR}" GREATER_EQUAL 6))
        message(STATUS "Generating the Single-Header")
        # Use python3 generate single header
        add_custom_command(OUTPUT "${SINGLE_HEADER}"
                COMMAND ${Python_EXECUTABLE} "${PROJECT_SOURCE_DIR}/scripts/merge.py"
                DEPENDS ${HEADERS}
                COMMENT "python3 scripts/merge.py"
                PRE_BUILD   # generate single header before any thing
                VERBATIM
        )
        set_source_files_properties("${SINGLE_HEADER}" PROPERTIES GENERATED TRUE)
        add_custom_target(single-header DEPENDS "${SINGLE_HEADER}")
    else ()
        message(WARNING "Requires Python 3.6 but only Python ${Python_VERSION} was found.")
    endif ()
else ()
    message(WARNING "Since the Python is not found, the test case for Single-Header will not be generated.")
endif ()

list(APPEND HEADERS "${ENTRY_HEADER}")

# -------------------------------------------------------------------------------
# Build examples
# -------------------------------------------------------------------------------

add_subdirectory(examples)

# -------------------------------------------------------------------------------
# Add tests
# -------------------------------------------------------------------------------

enable_testing()
add_subdirectory(tests)
