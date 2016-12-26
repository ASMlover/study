cmake_minimum_required(VERSION 3.0)
project(DEMO4)

option(USE_DMATH "use custom math implementation" ON)

if (USE_DMATH)
  include_directories("${PROJECT_SOURCE_DIR}/math")
  add_subdirectory(math)
  set(LIBS ${LIBS} math)
endif (USE_DMATH)

aux_source_directory(. SOURCES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -std=c++11")

add_executable(demo ${SOURCES})
target_link_libraries(demo ${LIBS})
