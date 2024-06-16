set(PREFIX example)

file(GLOB FILES ${CMAKE_CURRENT_SOURCE_DIR}/*)

set(TARGETS)

foreach (path ${FILES})
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

        # check if target exists
        list(FIND TARGETS "${PREFIX}-${file}" TARGET_INDEX)
        if (TARGET_INDEX GREATER -1)
            target_sources(${PREFIX}-${file} PUBLIC ${path})
            continue()
        endif ()

        # add executable
        add_executable(${PREFIX}-${file} ${path} ${HEADERS})
        list(APPEND TARGETS "${PREFIX}-${file}")
        message(STATUS "==[ Add ${PREFIX}: ${file}")
    endif ()
endforeach ()
