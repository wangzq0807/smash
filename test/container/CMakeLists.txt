
link_libraries(
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

set(BITMAP_SRC
    bitmap_test.cpp
    ../deps/log.cpp
    ${CMAKE_SOURCE_DIR}/kernel/lib/bitmap.c)

add_executable(bitmap_test ${BITMAP_SRC})

set(HASHMAP_SRC
    hashmap_test.cpp
    ../deps/log.cpp
    ${CMAKE_SOURCE_DIR}/kernel/lib/hashmap.c)

add_executable(hashmap_test ${HASHMAP_SRC})

set(LIST_SRC
    list_test.cpp
    ../deps/log.cpp
    ${CMAKE_SOURCE_DIR}/kernel/lib/list.c)

add_executable(list_test ${LIST_SRC})

set(BUDDY_SRC
    buddy_test.cpp
    ../deps/log.cpp
    ${CMAKE_SOURCE_DIR}/kernel/lib/bitmap.c
    ${CMAKE_SOURCE_DIR}/kernel/lib/buddy.c)

add_executable(buddy_test ${BUDDY_SRC})

