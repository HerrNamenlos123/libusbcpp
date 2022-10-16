
function(GET_GITHUB_DEPENDENCY NAME REQUIRED_FILE REPOSITORY_URL BRANCH)
    if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/${REQUIRED_FILE}")
        find_package(Git REQUIRED)
        if(GIT_FOUND)
            message(STATUS "${REQUIRED_FILE} not found, getting dependency '${NAME}'")
            file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/modules)
            set(GIT_COMMAND ${GIT_EXECUTABLE} clone ${REPOSITORY_URL} --single-branch --branch=${BRANCH} --depth=1)
            execute_process(COMMAND ${GIT_COMMAND}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/modules
                    RESULT_VARIABLE GIT_RESULT ERROR_VARIABLE GIT_OUTPUT OUTPUT_QUIET)
            if(NOT GIT_RESULT EQUAL "0")
                message(FATAL_ERROR "${GIT_COMMAND} failed. Error: '${GIT_RESULT}' Output: '${GIT_OUTPUT}'. Please check the dependencies")
            endif()
        else()
            message(FATAL_ERROR "Cannot get dependencies, git executable not found. Please install Git")
        endif()
    endif()
endfunction()

function(USE_STATIC_RUNTIME TARGET)
    if (MSVC)
        target_compile_options(${TARGET} PRIVATE /MT$<$<CONFIG:Debug>:d>)
    else()
        target_compile_options(${TARGET} PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++)
    endif()
endfunction()

function(SET_RUNTIME_OUTPUT_DIRECTORY TARGET PATH)
    set_target_properties(${TARGET} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY   "${PATH}/$<IF:$<CONFIG:DEBUG>,debug,release>"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${PATH}/debug"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PATH}/release"
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PATH}/minsizerel"
            RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PATH}/relwithdebinfo"
            )
endfunction()

function(SET_ARCHIVE_OUTPUT_DIRECTORY TARGET PATH)
    set_target_properties(${TARGET} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY   "${PATH}/$<IF:$<CONFIG:DEBUG>,debug,release>"
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${PATH}/debug"
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PATH}/release"
            ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${PATH}/minsizerel"
            ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PATH}/relwithdebinfo"
            )
endfunction()

function(SET_LIBRARY_OUTPUT_DIRECTORY TARGET PATH)
    set_target_properties(${TARGET} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY   "${PATH}/$<IF:$<CONFIG:DEBUG>,debug,release>"
            LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${PATH}/debug"
            LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PATH}/release"
            LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${PATH}/minsizerel"
            LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PATH}/relwithdebinfo"
            )
endfunction()