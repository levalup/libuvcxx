set(PREFIX test)

file(GLOB FILES ${CMAKE_CURRENT_SOURCE_DIR}/*)

set(TARGETS)

foreach (path ${FILES})
    if (IS_DIRECTORY "${path}")
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

        file(GLOB_RECURSE DIR_SRC "${path}/*.cxx" "${path}/*.cpp" "${path}/*.cc" "${path}/*.c")
        if ("${DIR_SRC}" STREQUAL "")
            continue()
        endif ()

        # check if target exists
        list(FIND TARGETS "${PREFIX}-${dir}" TARGET_INDEX)
        if (TARGET_INDEX GREATER -1)
            target_sources(${PREFIX}-${dir} PUBLIC ${DIR_SRC})
            continue()
        endif ()

        # add executable
        add_executable(${PREFIX}-${dir} ${DIR_SRC} ${HEADERS})
        list(APPEND TARGETS "${PREFIX}-${dir}")
        message(STATUS "==[ Add ${PREFIX}: ${dir}")
    else ()
        string(REGEX MATCH "[^/]*.[(c)|(cc)|(cpp)|(cxx)]$" file_ext "${path}")
        if ("${file_ext}" STREQUAL "")
            continue()
        endif ()
        string(REGEX MATCH "^[^.]*" file "${file_ext}")

        # skip single header if there is no that target
        if (NOT TARGET single-header AND "${PREFIX}-${file}" STREQUAL "test-single-header")
            continue()
        endif ()

        # check if target exists
        list(FIND TARGETS "${PREFIX}-${file}" TARGET_INDEX)
        if (TARGET_INDEX GREATER -1)
            target_sources(${PREFIX}-${file} PUBLIC "${path}")
            continue()
        endif ()

        # add executable
        add_executable(${PREFIX}-${file} "${path}" ${HEADERS})
        list(APPEND TARGETS "${PREFIX}-${file}")
        message(STATUS "==[ Add ${PREFIX}: ${file}")
    endif ()
endforeach ()

# explicitly add test-single-header target dependencies
if (TARGET single-header)
    add_dependencies(test-single-header single-header)
endif ()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    foreach (target ${TARGETS})
        set_target_properties(${target} PROPERTIES OUTPUT_NAME "debug-${target}")
    endforeach ()
endif ()

if (WALL)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        foreach (target ${TARGETS})
            target_compile_options(${target} PRIVATE -Wall -Wextra)
        endforeach ()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        foreach (target ${TARGETS})
            target_compile_options(${target} PRIVATE /W4)
        endforeach ()
    endif ()
endif ()

if (WERROR)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        foreach (target ${TARGETS})
            target_compile_options(${target} PRIVATE -Werror)
        endforeach ()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        foreach (target ${TARGETS})
            target_compile_options(${target} PRIVATE /WX)
        endforeach ()
    endif ()
endif ()

foreach (target ${TARGETS})
    string(REGEX MATCH "test-(.*)" name "${target}")
    set(name ${CMAKE_MATCH_1})
    add_test(NAME ${name} COMMAND ${target})
endforeach ()
