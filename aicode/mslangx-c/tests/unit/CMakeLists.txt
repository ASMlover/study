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

function(mslangc_add_frontend_unit_test test_name source_file test_label)
  add_executable("${test_name}" "${source_file}")
  target_link_libraries("${test_name}" PRIVATE mslang_frontend)
  mslangc_apply_test_warnings("${test_name}")
  add_test(NAME "${test_name}" COMMAND "${test_name}")
  set_tests_properties("${test_name}" PROPERTIES
    LABELS "${MSLANGC_TEST_LABEL_UNIT};${test_label}")
endfunction()

mslangc_add_support_unit_test(support.diag diag_test.c)
mslangc_add_support_unit_test(support.arena arena_test.c)
mslangc_add_support_unit_test(support.buffer buffer_test.c)
mslangc_add_runtime_unit_test(runtime_core.value value_test.c)
mslangc_add_runtime_unit_test(runtime_core.string string_test.c)
mslangc_add_runtime_unit_test(runtime_core.table table_test.c)
mslangc_add_frontend_unit_test(lexer.unit lexer_test.c lexer)
mslangc_add_frontend_unit_test(parser_expr.unit parser_expr_test.c parser_expr)

add_executable(lexer.fixture lexer_fixture_test.c)
target_link_libraries(lexer.fixture PRIVATE mslang_frontend)
mslangc_apply_test_warnings(lexer.fixture)

add_executable(parser_expr.fixture parser_expr_fixture_test.c)
target_link_libraries(parser_expr.fixture PRIVATE mslang_frontend)
mslangc_apply_test_warnings(parser_expr.fixture)

function(mslangc_add_lexer_fixture_test test_name fixture_name)
  add_test(NAME "${test_name}"
    COMMAND lexer.fixture
      "${CMAKE_CURRENT_SOURCE_DIR}/../ms/lexer/${fixture_name}")
  set_tests_properties("${test_name}" PROPERTIES
    LABELS "${MSLANGC_TEST_LABEL_INTEGRATION};lexer")
endfunction()

function(mslangc_add_parser_expr_fixture_test test_name fixture_name)
  add_test(NAME "${test_name}"
    COMMAND parser_expr.fixture
      "${CMAKE_CURRENT_SOURCE_DIR}/../ms/parser_expr/${fixture_name}")
  set_tests_properties("${test_name}" PROPERTIES
    LABELS "${MSLANGC_TEST_LABEL_INTEGRATION};parser_expr")
endfunction()

mslangc_add_lexer_fixture_test(lexer.fixture.basic_program basic_program.ms)
mslangc_add_lexer_fixture_test(lexer.fixture.all_keywords all_keywords.ms)
mslangc_add_lexer_fixture_test(lexer.fixture.invalid_char invalid_char.ms)
mslangc_add_lexer_fixture_test(lexer.fixture.unterminated_string unterminated_string.ms)

mslangc_add_parser_expr_fixture_test(parser_expr.fixture.precedence_assignment precedence_assignment.ms)
mslangc_add_parser_expr_fixture_test(parser_expr.fixture.postfix_and_literals postfix_and_literals.ms)
mslangc_add_parser_expr_fixture_test(parser_expr.fixture.function_and_super function_and_super.ms)