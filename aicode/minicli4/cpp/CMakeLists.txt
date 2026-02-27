cmake_minimum_required(VERSION 3.20)
project(minicli4_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(MINICLI4_ENABLE_FTXUI "Enable FTXUI backend when available" ON)

add_library(minicli4_cpp_lib
  src/agent.cpp
  src/cli.cpp
  src/commands.cpp
  src/completion.cpp
  src/config.cpp
  src/provider.cpp
  src/session.cpp
  src/tools.cpp
  src/tui.cpp
)
target_include_directories(minicli4_cpp_lib PUBLIC include)

if(MINICLI4_ENABLE_FTXUI)
  find_package(ftxui QUIET)
  if(ftxui_FOUND)
    target_link_libraries(minicli4_cpp_lib PUBLIC ftxui::screen ftxui::dom ftxui::component)
    target_compile_definitions(minicli4_cpp_lib PUBLIC MINICLI4_HAVE_FTXUI=1)
    message(STATUS "MiniCLI4 C++: FTXUI backend enabled")
  else()
    message(STATUS "MiniCLI4 C++: FTXUI not found, using ANSI fallback backend")
  endif()
endif()

add_executable(minicli4-cpp src/main.cpp)
target_link_libraries(minicli4-cpp PRIVATE minicli4_cpp_lib)

enable_testing()
add_executable(minicli4_cpp_tests tests/test_main.cpp)
target_link_libraries(minicli4_cpp_tests PRIVATE minicli4_cpp_lib)
add_test(NAME minicli4_cpp_tests COMMAND minicli4_cpp_tests)
