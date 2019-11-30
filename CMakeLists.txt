cmake_minimum_required (VERSION 3.0)

# include_directories (include)

add_subdirectory(boot)
add_subdirectory(kernel)
add_subdirectory(usr)
add_subdirectory(test)

# 安装bootloader,内核,应用
add_custom_target (fs)
add_custom_command (TARGET fs PRE_BUILD
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tools/install.sh ${CMAKE_BINARY_DIR}  ${CMAKE_BINARY_DIR}
    )

add_custom_target(run)
add_custom_command (TARGET run PRE_BUILD
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tools/install.sh ${CMAKE_BINARY_DIR}  ${CMAKE_BINARY_DIR}
    COMMAND bochs -q -f ${CMAKE_BINARY_DIR}/bochsrc
    )
