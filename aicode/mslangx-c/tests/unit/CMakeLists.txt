function(mslangc_add_support_unit_test test_name source_file)
  add_executable("${test_name}" "${source_file}")
  target_link_libraries("${test_name}" PRIVATE mslang_support)

  if(MSVC)
    target_compile_options("${test_name}" PRIVATE /W4 /WX)
  else()
    target_compile_options("${test_name}" PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()

  add_test(NAME "${test_name}" COMMAND "${test_name}")
  set_tests_properties("${test_name}" PROPERTIES
    LABELS "${MSLANGC_TEST_LABEL_UNIT};support")
endfunction()

function(mslangc_add_runtime_unit_test test_name source_file)
  add_executable("${test_name}" "${source_file}")
  target_link_libraries("${test_name}" PRIVATE mslang_runtime_core)

  if(MSVC)
    target_compile_options("${test_name}" PRIVATE /W4 /WX)
  else()
    target_compile_options("${test_name}" PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()

  add_test(NAME "${test_name}" COMMAND "${test_name}")
  set_tests_properties("${test_name}" PROPERTIES
    LABELS "${MSLANGC_TEST_LABEL_UNIT};runtime_core")
endfunction()

mslangc_add_support_unit_test(support.diag diag_test.c)
mslangc_add_support_unit_test(support.arena arena_test.c)
mslangc_add_support_unit_test(support.buffer buffer_test.c)
mslangc_add_runtime_unit_test(runtime_core.value value_test.c)
mslangc_add_runtime_unit_test(runtime_core.string string_test.c)
mslangc_add_runtime_unit_test(runtime_core.table table_test.c)