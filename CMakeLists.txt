# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15)

project(nutrition_calculator C CXX)

macro(setup_module)
	# Utils dependency
	set(UTILS_DIR "${DEPS}/Utils")

	# Utils dependency
	set(UTILS_DIR "${DEPS}/Utils")
	file(GLOB_RECURSE UTILS_HEADERS "${UTILS_DIR}/include/*")
	# set(EXTERNAL_HEADERS ${EXTERNAL_HEADERS}
	# 	${UTILS_HEADERS}
	# )
	GROUP_SRC("${UTILS_HEADERS}" "${UTILS_DIR}/include")
	module_message("UTILS_DIR: ${UTILS_DIR}")

	# DMBCore dependency
	add_dependency_module(${DEPS}/DataModelBuilder/Core DMBCore)

	# Networking dependency
	add_dependency_module(${DEPS}/Networking/http http)

	add_src(
		main.cpp
		${UTILS_DIR}/src/networking/uploader_with_auth.cpp
		${UTILS_DIR}/src/networking/downloader_with_auth.cpp
		${UTILS_DIR}/src/networking/downloader_with_version_control.cpp
		${UTILS_DIR}/include/utils/networking/sync_resources.h
		${UTILS_DIR}/src/string_utils.cpp
		${UTILS_DIR}/src/file_utils.cpp
		${UTILS_DIR}/src/io_utils.cpp
		${UTILS_DIR}/src/datetime.cpp
		${UTILS_DIR}/src/envar.cpp
	)

	# For IDE
	add_headers_to_sources()

	# Add executable
	module_add_library_or_executable()

	# if (WIN32)
	# 	add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
	# 		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${MODULE_NAME}> $<TARGET_FILE_DIR:${MODULE_NAME}>
	# 		COMMAND_EXPAND_LISTS
	# 	)
	# endif()
endmacro()

include(cmake_modules/modules.cmake)
modules_include_all(cmake_modules)
setup_module_project()
