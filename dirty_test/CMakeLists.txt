project("dirty_test")
add_executable(dirty_tester dirty_test.cpp)
target_link_libraries(dirty_tester gtest_main gtest sqlite3 pthread)

