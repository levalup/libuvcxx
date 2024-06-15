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
file(GLOB TESTS ${PROJECT_SOURCE_DIR}/test/*)

set(TARGETS)

foreach (path ${TESTS})
    if (IS_DIRECTORY ${path})
        string(REGEX MATCH "[^/]*$" dir "${path}")

        # check ignore
        set(NEED_IGNORE 0)
        foreach (pattern ${IGNORE})
            string(REGEX MATCH "^${pattern}$" matched "${dir}")
            if (NOT "${matched}" STREQUAL "")
                set(NEED_IGNORE 1)
                break()
            endif ()
        endforeach ()
        if (${NEED_IGNORE})
            message(STATUS "==[ Ignore: ${dir}")
            continue()
        endif ()

        file(GLOB_RECURSE DIR_SRC ${path}/*.cxx ${path}/*.cpp ${path}/*.cc ${path}/*.c)
        if ("${DIR_SRC}" STREQUAL "")
            continue()
        endif ()

        # check if target exists
        list(FIND TARGETS "${dir}" TARGET_INDEX)
        if(TARGET_INDEX GREATER -1)
            target_sources(${dir} PUBLIC ${DIR_SRC})
            continue()
        endif()

        # add executable
        add_executable(${dir} ${DIR_SRC} ${HEADERS})
        list(APPEND TARGETS ${dir})
        message(STATUS "==[ Add executable: ${dir}")
    else ()
        string(REGEX MATCH "[^/]*.[(c)|(cc)|(cpp)|(cxx)]$" file_ext "${path}")
        if ("${file_ext}" STREQUAL "")
            continue()
        endif ()
        string(REGEX MATCH "^[^.]*" file "${file_ext}")

        # check if target exists
        list(FIND TARGETS "${file}" TARGET_INDEX)
        if(TARGET_INDEX GREATER -1)
            target_sources(${file} PUBLIC ${path})
            continue()
        endif()

        # add executable
        add_executable(${file} ${path} ${HEADERS})
        list(APPEND TARGETS ${file})
        message(STATUS "==[ Add executable: ${file}")
    endif ()
endforeach ()
