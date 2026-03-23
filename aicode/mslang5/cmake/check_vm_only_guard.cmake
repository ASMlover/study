if(NOT DEFINED ROOT_DIR)
  message(FATAL_ERROR "ROOT_DIR is required")
endif()

set(removed_legacy_files
  "${ROOT_DIR}/src/runtime/script_interpreter.cc"
  "${ROOT_DIR}/src/runtime/script_interpreter.hh"
)

foreach(legacy_file IN LISTS removed_legacy_files)
  if(EXISTS "${legacy_file}")
    message(FATAL_ERROR
      "VM-only guard failed.\n"
      "Legacy interpreter file must not exist:\n"
      "  ${legacy_file}\n"
      "Legacy interpreter reintroduction is not allowed in the mainline runtime path.")
  endif()
endforeach()

set(scan_files
  "${ROOT_DIR}/CMakeLists.txt"
)

file(GLOB_RECURSE runtime_source_files
  "${ROOT_DIR}/src/*.cc"
  "${ROOT_DIR}/src/*.hh"
)
list(APPEND scan_files ${runtime_source_files})

set(banned_patterns
  "MAPLE_ENABLE_LEGACY_INTERPRETER"
  "ScriptInterpreter"
  "kVmCompileFailedThenLegacy"
  "kVmPreferredWithLegacyFallback"
  "kLegacyOnly"
)

foreach(scan_file IN LISTS scan_files)
  file(READ "${scan_file}" file_content)
  foreach(pattern IN LISTS banned_patterns)
    string(FIND "${file_content}" "${pattern}" hit_index)
    if(NOT hit_index EQUAL -1)
      message(FATAL_ERROR
        "VM-only guard failed.\n"
        "Forbidden legacy token found:\n"
        "  token: ${pattern}\n"
        "  file:  ${scan_file}\n"
        "Remove legacy fallback symbols from runtime and build definitions.")
    endif()
  endforeach()
endforeach()

message(STATUS "VM-only guard passed")
