

set(BITMAP_SRC
    bitmap_test.cpp
    ../deps/log.cpp
    ${CMAKE_SOURCE_DIR}/kernel/lib/bitmap.c)

add_executable(bitmap_test ${BITMAP_SRC})

target_link_libraries(bitmap_test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

set(HASHMAP_SRC
    hash_test.cpp
    ../deps/log.cpp
    ${CMAKE_SOURCE_DIR}/kernel/lib/hash_test.c)

add_executable(hashmap_test ${BITMAP_SRC})

target_link_libraries(hashmap_test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )
