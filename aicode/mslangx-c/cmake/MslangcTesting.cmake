set(MSLANGC_TEST_LABEL_SMOKE "smoke")
set(MSLANGC_TEST_LABEL_UNIT "unit")
set(MSLANGC_TEST_LABEL_INTEGRATION "integration")
set(MSLANGC_TEST_LABEL_CONFORMANCE "conformance")
set(MSLANGC_TEST_LABEL_STRESS "stress")

function(mslangc_add_smoke_test test_name)
	add_test(NAME "${test_name}" COMMAND "${CMAKE_COMMAND}" ${ARGN})
	set_tests_properties("${test_name}" PROPERTIES
		LABELS "${MSLANGC_TEST_LABEL_SMOKE}")
endfunction()