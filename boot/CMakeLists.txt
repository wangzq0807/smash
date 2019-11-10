add_subdirectory(mbr)
add_subdirectory(loader)

# 安装mbr和bootloader
add_custom_target (boot_install ALL)
add_dependencies (boot_install loader mbr)
add_custom_command (TARGET boot_install PRE_BUILD
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tools/install.sh ${CMAKE_BINARY_DIR}/boot  ${CMAKE_BINARY_DIR}
    )

