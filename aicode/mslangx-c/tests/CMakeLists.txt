mslangc_add_smoke_test(smoke.help
  -DMSLANGC_EXE="$<TARGET_FILE:mslangc>"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/smoke_help_test.cmake")

add_subdirectory(unit)