# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project (nutrition_calculator C CXX)

SET(MY_FLAGS "/std:c++latest")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_FLAGS}")

if (LOG_ON)
	add_compile_definitions("LOG_ON")
endif ()

#include_directories(".")

if ("${DEPS}" STREQUAL "")
	message("nutrition_calculator: WARNING: DEPS is not specified. Take the default value")
	set(DEPS "${PROJECT_SOURCE_DIR}/dependencies")
else()
	message("nutrition_calculator: DEPS: ${DEPS}")
endif()

set(UTILS_DIR "${DEPS}/Utils/include")
foreach(items ${UTILS_DIR})
	file(GLOB_RECURSE UTILS_SRC "${items}/*.cpp")
endforeach()

set(nutrition_calculator_INCLUDES
	"${PROJECT_SOURCE_DIR}/include"
	"${UTILS_DIR}"
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

set_target_properties(nutrition_calculator PROPERTIES LINKER_LANGUAGE CXX)