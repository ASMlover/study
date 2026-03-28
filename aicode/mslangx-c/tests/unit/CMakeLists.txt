function(mslangc_apply_test_warnings target_name)
  if(MSVC)
    target_compile_options("${target_name}" PRIVATE /W4 /WX)
  else()
    target_compile_options("${target_name}" PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
endfunction()

function(mslangc_add_support_unit_test test_name source_file)
  add_executable("${test_name}" "${source_file}")
  target_link_libraries("${test_name}" PRIVATE mslang_support)
  mslangc_apply_test_warnings("${test_name}")
  add_test(NAME "${test_name}" COMMAND "${test_name}")
  set_tests_properties("${test_name}" PROPERTIES
    LABELS "${MSLANGC_TEST_LABEL_UNIT};support")
endfunction()

function(mslangc_add_runtime_unit_test test_name source_file)
  add_executable("${test_name}" "${source_file}")
  target_link_libraries("${test_name}" PRIVATE mslang_runtime_core)
  mslangc_apply_test_warnings("${test_name}")
  add_test(NAME "${test_name}" COMMAND "${test_name}")
  set_tests_properties("${test_name}" PROPERTIES
    LABELS "${MSLANGC_TEST_LABEL_UNIT};runtime_core")
endfunction()

function(mslangc_add_frontend_unit_test test_name source_file)
  add_executable("${test_name}" "${source_file}")
  target_link_libraries("${test_name}" PRIVATE mslang_frontend)
  mslangc_apply_test_warnings("${test_name}")
  add_test(NAME "${test_name}" COMMAND "${test_name}")
  set_tests_properties("${test_name}" PROPERTIES
    LABELS "${MSLANGC_TEST_LABEL_UNIT};lexer")
endfunction()

mslangc_add_support_unit_test(support.diag diag_test.c)
mslangc_add_support_unit_test(support.arena arena_test.c)
mslangc_add_support_unit_test(support.buffer buffer_test.c)
mslangc_add_runtime_unit_test(runtime_core.value value_test.c)
mslangc_add_runtime_unit_test(runtime_core.string string_test.c)
mslangc_add_runtime_unit_test(runtime_core.table table_test.c)
mslangc_add_frontend_unit_test(lexer.unit lexer_test.c)

add_executable(lexer.fixture lexer_fixture_test.c)
target_link_libraries(lexer.fixture PRIVATE mslang_frontend)
mslangc_apply_test_warnings(lexer.fixture)

function(mslangc_add_lexer_fixture_test test_name fixture_name)
  add_test(NAME "${test_name}"
    COMMAND lexer.fixture
      "${CMAKE_CURRENT_SOURCE_DIR}/../ms/lexer/${fixture_name}")
  set_tests_properties("${test_name}" PROPERTIES
    LABELS "${MSLANGC_TEST_LABEL_INTEGRATION};lexer")
endfunction()

mslangc_add_lexer_fixture_test(lexer.fixture.basic_program basic_program.ms)
mslangc_add_lexer_fixture_test(lexer.fixture.all_keywords all_keywords.ms)
mslangc_add_lexer_fixture_test(lexer.fixture.invalid_char invalid_char.ms)
mslangc_add_lexer_fixture_test(lexer.fixture.unterminated_string unterminated_string.ms)