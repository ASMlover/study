project(public_logic)

message(STATUS "Configuration public_logic")

file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SOURCE_FILES})

neox_check_threads()
neox_check_macro(PUBLIC_LOGIC)

set(_TARGET_DEPENDS c_common)
neox_merge_target_depends(_MERGED_DEPENDS _TARGET_DEPENDS)

set(_THIRD_DEPENDS PYTHON25 BOOST)
neox_merge_3rd_depends(_THIRD_DEPENDS _MERGED_DEPENDS)
neox_config_3rd_include_macro(_THIRD_DEPENDS)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	"${CMAKE_CURRENT_SOURCE_DIR}/../"
	"${CMAKE_CURRENT_SOURCE_DIR}/../.."
)
neox_set_device_include()

add_library(${PROJECT_NAME} ${NEOX_INNER_LINK} ${HEADER_FILES} ${SOURCE_FILES})
if (${NEOX_PLATFORM} STREQUAL ${NEOX_WIN32})
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".pyd")
elseif (${NEOX_PLATFORM} STREQUAL ${NEOX_MAC})
	set_target_properties(${PROJECT_NAME} PROPERTIES
		BUILD_WITH_INSTALL_RPATH ON
		INSTALL_NAME_DIR "@executable_path/../dylib")
endif()

neox_set_third_property(${PROJECT_NAME} _THIRD_DEPENDS)
neox_set_depend_property(${PROJECT_NAME} _MERGED_DEPENDS)
neox_link_device(${PROJECT_NAME})
neox_set_output_dir(${PROJECT_NAME} engine)

