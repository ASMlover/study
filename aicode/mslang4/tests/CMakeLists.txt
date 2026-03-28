add_executable(test_common unit/test_common.c)
target_include_directories(test_common PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME test_common COMMAND test_common)

add_executable(test_memory unit/test_memory.c ${CMAKE_SOURCE_DIR}/src/memory.c)
target_include_directories(test_memory PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME test_memory COMMAND test_memory)
