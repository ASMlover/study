if(NOT DEFINED MSLANGC_EXE)
  message(FATAL_ERROR "MSLANGC_EXE is not set")
endif()

string(REGEX REPLACE "^\"(.*)\"$" "\\1" MSLANGC_EXE "${MSLANGC_EXE}")

if(NOT EXISTS "${MSLANGC_EXE}")
  message(FATAL_ERROR "mslangc executable not found: ${MSLANGC_EXE}")
endif()

execute_process(
  COMMAND "${MSLANGC_EXE}" --help
  RESULT_VARIABLE help_exit_code
  OUTPUT_VARIABLE help_stdout
  ERROR_VARIABLE help_stderr)

if(NOT help_exit_code EQUAL 0)
  message(FATAL_ERROR
    "mslangc --help exited with ${help_exit_code}\n"
    "stdout:\n${help_stdout}\n"
    "stderr:\n${help_stderr}")
endif()

if(help_stdout STREQUAL "")
  message(FATAL_ERROR "mslangc --help did not print usage text")
endif()