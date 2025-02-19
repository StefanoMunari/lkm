function(fetchcontent NAME LOCAL_PATH REMOTE_PATH GIT_HASH CACHE_CONTENT FORCE_LOCAL)
    include(FetchContent)
    # get base dir using a regex
    string(REGEX REPLACE "^file://(/.*/)[^/]+$" "\\1" BASE_PATH "${LOCAL_PATH}")
    # use base dir for all fetched content
    set(FETCHCONTENT_BASE_DIR "${BASE_PATH}")
    # do not update already downloaded content
    if (${CACHE_CONTENT})
        set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
    endif ()
    if (NOT ${FORCE_LOCAL})
        # check if server is reachable
        execute_process(
                COMMAND ping github.com -c 2 -w 1000
                RESULT_VARIABLE NO_CONNECTION
        )
    endif ()
    # use local fetched content if no connection
    if (${FORCE_LOCAL} OR NO_CONNECTION GREATER 0)
        set(PATH "${LOCAL_PATH}")
        message(NOTICE "Fetching from local path: ${LOCAL_PATH}")
    else ()# fetch from remote path if connection available and no-caching enabled
        set(PATH "${REMOTE_PATH}")
        message(NOTICE "Fetching from remote path: ${REMOTE_PATH}")
    endif ()
    FetchContent_Declare(
            ${NAME}
            GIT_REPOSITORY ${PATH}
            GIT_TAG ${GIT_HASH}
    )
    FetchContent_MakeAvailable(${NAME})
endfunction()