add_executable(cpp_iterator_test cpp_iterator_test.cc)
target_link_libraries(cpp_iterator_test gtest_main gmock_main)
add_test(
  NAME cpp_iterator_test
  COMMAND ${CMAKE_BINARY_DIR}/bin/cpp_iterator_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
