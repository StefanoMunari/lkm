set(KERNEL_TARGET_NAME "event")
set(EVENT_DIR "${CMAKE_SOURCE_DIR}/${KERNEL_TARGET_NAME}")
# commands to enable Clion syntax highlighting
set(KERNEL_SYNTAX_HEADERS "${KERNEL_TARGET_NAME}_empty")
set(USER_TARGET_NAME "${KERNEL_TARGET_NAME}_usr")

add_executable(${KERNEL_SYNTAX_HEADERS}
        ${EVENT_DIR}/event_dev.c
)
add_executable(${USER_TARGET_NAME}
        ${EVENT_DIR}/event_userspace.c
)
target_include_directories(${KERNEL_SYNTAX_HEADERS}
        PRIVATE ${KERNELHEADERS_DIR}
)
# Create a build target that calls Makefile
add_custom_target(build_${KERNEL_TARGET_NAME} ALL
        COMMAND make -C ${EVENT_DIR}
        WORKING_DIRECTORY ${EVENT_DIR}
        COMMENT "Build ${KERNEL_TARGET_NAME} module"
)
# Create a clean target
add_custom_target(clean_${KERNEL_TARGET_NAME}
        COMMAND make -C ${EVENT_DIR} clean
        WORKING_DIRECTORY ${EVENT_DIR}
        COMMENT "Clean ${KERNEL_TARGET_NAME} module"
)