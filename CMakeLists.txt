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
#include_directories(".")

if ("${DEPS}" STREQUAL "")
	message("nutrition_calculator: WARNING: DEPS is not specified. Take the default value")
	set(DEPS "$ENV{nutrition_calculator_deps}")
else()
	message("nutrition_calculator: DEPS: ${DEPS}")
endif()

# Include Utils library
set(UTILS_DIR "${DEPS}/Utils")
add_subdirectory ("${UTILS_DIR}" "${UTILS_DIR}/Build-cmake")

set(CUSTOM_INCLUDES ${CUSTOM_INCLUDES} ${utils_INCLUDES})

# Build directory name
if (CMAKE_CXX_COMPILER EQUAL "g++")
	set(BUILD_DIR_NAME "Build-g++")
else ()
	set(BUILD_DIR_NAME "Build-cmake")
endif ()

# Networking dependency
if ("${NETLIB_DIR}" STREQUAL "")
	set(NETLIB_DIR "${DEPS}/Networking/netlib")
else()
	message("NETLIB_DIR: ${NETLIB_DIR}")
endif()

add_subdirectory("${NETLIB_DIR}" "${NETLIB_DIR}/${BUILD_DIR_NAME}")

# DMBCore dependency
if ("${DMBCORE_DIR}" STREQUAL "")
	set(DMBCORE_DIR "${DEPS}/DataModelBuilder/Core")
else()
	message("DMBCORE_DIR: ${DMBCORE_DIR}")
endif()

if (CMAKE_CXX_COMPILER EQUAL "g++")
	set(BUILD_DIR_NAME "Build-g++")
else ()
	set(BUILD_DIR_NAME "Build-cmake")
endif ()

add_subdirectory("${DMBCORE_DIR}" "${DMBCORE_DIR}/${BUILD_DIR_NAME}")

set(nutrition_calculator_INCLUDES
	"${PROJECT_SOURCE_DIR}/include"
	"${CUSTOM_INCLUDES}"
	"${NETLIB_INCLUDES}"
	"${DMBCORE_INCLUDES}"
)

if (NOT "${INCLUDES}" STREQUAL "")
	message("nutrition_calculator: External INCLUDES variable: ${INCLUDES}")
	set(nutrition_calculator_INCLUDES ${nutrition_calculator_INCLUDES} ${INCLUDES})
	foreach(items ${INCLUDES})
		file(GLOB_RECURSE INCLUDES_SRC "${items}/*.cpp")
	endforeach()
	message("nutrition_calculator: External INCLUDES sources: ${INCLUDES_SRC}")
endif()

set(nutrition_calculator_INCLUDES ${nutrition_calculator_INCLUDES} PARENT_SCOPE)
message("nutrition_calculator: nutrition_calculator_INCLUDES: ${nutrition_calculator_INCLUDES}")

include_directories("${nutrition_calculator_INCLUDES}")
#include_directories(${PROJECT_BINARY_DIR})

file(GLOB SRC "src/*")
file(GLOB HEADERS ${HEADERS} "include/*")
set(SRC ${SRC} ${UTILS_SRC} ${INCLUDES_SRC} main.cpp)
message("'nutrition_calculator' project SRC: ${SRC}")
message("'nutrition_calculator' project HEADERS: ${HEADERS}")

add_executable (nutrition_calculator ${HEADERS} ${SRC})
target_link_libraries(nutrition_calculator utils)

target_link_libraries (nutrition_calculator netlib)
target_link_libraries (nutrition_calculator DMBCore)

# add_custom_command(
#     TARGET nutrition_calculator PRE_BUILD
#     COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/../get_dependencies.sh ${CMAKE_BUILD_TYPE} "${CMAKE_CURRENT_SOURCE_DIR}/.."
#     VERBATIM
# )

set_target_properties(nutrition_calculator PROPERTIES LINKER_LANGUAGE CXX)
