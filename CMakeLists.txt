# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project (nutrition_calculator C CXX)

if(MSVC)
	# fix __cplusplus macro for MSVC
	add_compile_options(
		"/Zc:__cplusplus"
		"/std:c++latest"
	)
else()
	set(CMAKE_CXX_STANDARD 20)
endif()

if (LOG_ON)
	add_compile_definitions("LOG_ON")
	message("nutrition_calculator: LOG_ON is defined")
else()
	message("nutrition_calculator: LOG_ON is not defined")
endif ()
if (LOG_LEVEL_VERBOSE)
	add_compile_definitions("LOG_LEVEL_VERBOSE")
	message("nutrition_calculator: LOG_LEVEL_VERBOSE is defined")
else()
	message("nutrition_calculator: LOG_LEVEL_VERBOSE is not defined")
endif ()
if (LOG_LEVEL_DEBUG)
	add_compile_definitions("LOG_LEVEL_DEBUG")
	message("nutrition_calculator: LOG_LEVEL_DEBUG is defined")
else()
	message("nutrition_calculator: LOG_LEVEL_DEBUG is not defined")
endif()

# Inherit parent libs logic
set(PARENT_LIBS ${PARENT_LIBS} ${LIBS})
set(PARENT_LIBS ${PARENT_LIBS} PARENT_SCOPE)
if (nutrition_calculator_INHERIT_PARENT_LIBS)
	set(LIBS ${PARENT_LIBS})
	if (nutrition_calculator_CMAKE_VERBOSE)
		message("nutrition_calculator PARENT_LIBS: ${PARENT_LIBS}")
	endif()
endif()
# End of Inherit parent libs logic

# Build directory name
if (CMAKE_CXX_COMPILER EQUAL "g++")
	set(BUILD_DIR_NAME "Build-g++")
else ()
	set(BUILD_DIR_NAME "Build-cmake")
endif ()

# set deps variables
set(nutrition_calculator_DEPS "$ENV{nutrition_calculator_deps}")
set(nutrition_calculator_DEPS "${nutrition_calculator_DEPS}" PARENT_SCOPE)
if ("${DEPS}" STREQUAL "")
	set(DEPS ${nutrition_calculator_DEPS})
endif()
if ("${nutrition_calculator_DEPS}" STREQUAL "")
	set(DEPS "$ENV{build_deps}")
endif()
if ("${DEPS}" STREQUAL "")
	message(SEND_ERROR "Project: ERROR: DEPS is not specified. Take the default value")
	set(DEPS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)
else()
	message("Project: DEPS: ${DEPS}")
endif()
set(DEPS ${DEPS} PARENT_SCOPE)

# Networking dependency
if ("${NETLIB_DIR}" STREQUAL "")
	set(NETLIB_DIR "${DEPS}/Networking/netlib")
else()
	message("NETLIB_DIR: ${NETLIB_DIR}")
endif()
add_subdirectory("${NETLIB_DIR}" "${NETLIB_DIR}/${BUILD_DIR_NAME}")
set(CUSTOM_INCLUDES ${CUSTOM_INCLUDES} ${NETLIB_INCLUDES})

# DMBCore dependency
if ("${DMBCORE_DIR}" STREQUAL "")
	set(DMBCORE_DIR "${DEPS}/DataModelBuilder/Core")
else()
	message("DMBCORE_DIR: ${DMBCORE_DIR}")
endif()
add_subdirectory("${DMBCORE_DIR}" "${DMBCORE_DIR}/${BUILD_DIR_NAME}")
set(CUSTOM_INCLUDES ${CUSTOM_INCLUDES} ${DMBCORE_INCLUDES})

# Set and add external includes
if (NOT "${INCLUDES}" STREQUAL "")
	message("Project: Add extra includes: ${INCLUDES}")
else()
	set(INCLUDES $ENV{nutrition_calculator_includes})
endif()

# Set nutrition_calculator_INCLUDES global variable
set(nutrition_calculator_INCLUDES ${nutrition_calculator_INCLUDES}
	"${PROJECT_SOURCE_DIR}/include"
	"${CUSTOM_INCLUDES}"
	"${INCLUDES}"
)
set(nutrition_calculator_INCLUDES ${nutrition_calculator_INCLUDES} PARENT_SCOPE)
message("nutrition_calculator: nutrition_calculator_INCLUDES: ${nutrition_calculator_INCLUDES}")

# include all directories
#include_directories(".")
#include_directories(${PROJECT_BINARY_DIR})
include_directories("${nutrition_calculator_INCLUDES}")

# Fetch CPP from external includes
if (NOT "${INCLUDES}" STREQUAL "")
	message("nutrition_calculator: External INCLUDES variable: ${INCLUDES}")
	foreach(items ${INCLUDES})
		file(GLOB_RECURSE INCLUDES_SRC "${items}/*.cpp")
	endforeach()
	message("nutrition_calculator: External INCLUDES sources: ${INCLUDES_SRC}")
endif()

# set up sources
file(GLOB SRC "src/*")
file(GLOB HEADERS ${HEADERS} "include/*")
set(SRC ${SRC} ${INCLUDES_SRC} main.cpp)

# Inherit parent sources logic
set(PARENT_SRC ${PARENT_SRC} ${SRC})
set(PARENT_SRC ${PARENT_SRC} PARENT_SCOPE)
if (nutrition_calculator_INHERIT_PARENT_SRC)
	set(SRC ${PARENT_SRC})
	if (nutrition_calculator_CMAKE_VERBOSE)
		message("nutrition_calculator PARENT_SRC: ${PARENT_SRC}")
	endif()
endif()
# End of Inherit parent sources logic

# check sources
message("'nutrition_calculator' project SRC: ${SRC}")
message("'nutrition_calculator' project HEADERS: ${HEADERS}")

# add executable
add_executable (nutrition_calculator ${HEADERS} ${SRC})

# link libraries
target_link_libraries (nutrition_calculator netlib)
target_link_libraries (nutrition_calculator DMBCore)

# Link all librariles including inherited
set(cnt 1)
foreach(item ${LIBS})
	if (${cnt} EQUAL 1)
		if (nutrition_calculator_CMAKE_VERBOSE)	
			message("Link library '${item}'")
		endif()
		set(lib_name ${item})
	elseif(${cnt} EQUAL 2)
		if (nutrition_calculator_CMAKE_VERBOSE)
			message("Library '${lib_name}' path: ${item}")
		endif()
		set(lib_path ${item})
	elseif(${cnt} EQUAL 3)
		if (nutrition_calculator_CMAKE_VERBOSE)
			message("Library '${lib_name}' binary path: ${item}")
		endif()
		#add_subdirectory("${lib_path}" "${item}")
		target_link_libraries(nutrition_calculator ${lib_name})
	endif()
	MATH(EXPR cnt "${cnt}+1")
endforeach()

# add_custom_command(
#     TARGET nutrition_calculator PRE_BUILD
#     COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/external_config.sh ${CMAKE_BUILD_TYPE} "${CMAKE_CURRENT_SOURCE_DIR}/.."
#     VERBATIM
# )

set_target_properties(nutrition_calculator PROPERTIES LINKER_LANGUAGE CXX)
