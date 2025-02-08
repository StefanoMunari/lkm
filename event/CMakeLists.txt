set(EVENT_DIR "${CMAKE_SOURCE_DIR}/event")
# commands to enable Clion syntax highlighting
set(KERNEL_SYNTAX_HEADERS "empty")
set(USER_TARGET_NAME "event_usr")

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
add_custom_target(build_kernel_module ALL
        COMMAND make -C ${EVENT_DIR}
        WORKING_DIRECTORY ${EVENT_DIR}
        COMMENT "Build the kernel module"
)
# Create a clean target
add_custom_target(clean_kernel_module
        COMMAND make -C ${EVENT_DIR} clean
        WORKING_DIRECTORY ${EVENT_DIR}
        COMMENT "Clean the kernel module"
)