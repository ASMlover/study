if(NOT DEFINED SOURCE_FILE)
  message(FATAL_ERROR "SOURCE_FILE is required")
endif()

if(NOT DEFINED EXPECTED_FILE)
  message(FATAL_ERROR "EXPECTED_FILE is required")
endif()

file(SHA256 "${SOURCE_FILE}" current_sha)
file(READ "${EXPECTED_FILE}" expected_sha)
string(STRIP "${expected_sha}" expected_sha)
string(TOLOWER "${current_sha}" current_sha)
string(TOLOWER "${expected_sha}" expected_sha)

if(NOT current_sha STREQUAL expected_sha)
  message(FATAL_ERROR
    "Script interpreter freeze guard failed.\n"
    "  source:   ${SOURCE_FILE}\n"
    "  expected: ${expected_sha}\n"
    "  actual:   ${current_sha}\n"
    "Interpreter path is frozen in M0. If this change is intentional, update docs/migration/interpreter_freeze.sha256 and docs/migration/m0-baseline-freeze.md together.")
endif()
