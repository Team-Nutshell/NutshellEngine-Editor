cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
set(NAME NutshellEngine-Scripts)
project(${NAME} VERSION 0.0.1 LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NTSHENGN_DEFINES "")
set(NTSHENGN_LIBRARIES "")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	list(APPEND NTSHENGN_LIBRARIES ${CMAKE_DL_LIBS})
endif()

set(NTSHENGN_COMMON_PATH "NtshEngn_Common_NOTFOUND" CACHE PATH "The directory containing NutshellEngine's common resources.")
if(NOT ${NTSHENGN_COMMON_PATH} STREQUAL "NtshEngn_Common_NOTFOUND" AND EXISTS ${NTSHENGN_COMMON_PATH})
	message(STATUS "Copy NutshellEngine\'s common resources from ${NTSHENGN_COMMON_PATH} to ${CMAKE_CURRENT_SOURCE_DIR}/Common")
	file(COPY ${NTSHENGN_COMMON_PATH}/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/Common)
	
	add_subdirectory(Common)
else()
	message(STATUS "Copy NutshellEngine\'s common resources from Git repository to ${CMAKE_CURRENT_SOURCE_DIR}/Common")
	FetchContent_Declare(
		Common
		GIT_REPOSITORY https://github.com/Team-Nutshell/NutshellEngine-Common
		GIT_TAG main
		SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common"
	)

	FetchContent_MakeAvailable(Common)
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_script_manager.py ${CMAKE_CURRENT_SOURCE_DIR})

set(SCRIPTS_DIRECTORY "scripts")
file(GLOB_RECURSE SCRIPTS "${SCRIPTS_DIRECTORY}/*.h")
message(STATUS ${SCRIPTS})

set(SCRIPT_MANAGER
	${SCRIPTS_DIRECTORY}/ntshengn_script_manager.h
	${SCRIPTS_DIRECTORY}/ntshengn_script_manager.cpp)

add_library(${PROJECT_NAME} SHARED ${SCRIPT_MANAGER} ${SCRIPTS})

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		NutshellEngine::Common
		${NTSHENGN_LIBRARIES})

target_compile_options(${PROJECT_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		${NTSHENGN_DEFINES}
	PUBLIC
		$<$<CONFIG:Debug>:NTSHENGN_DEBUG>
		$<$<CONFIG:MinSizeRel>:NTSHENGN_RELEASE>
		$<$<CONFIG:Release>:NTSHENGN_RELEASE>
		$<$<CONFIG:RelWithDebInfo>:NTSHENGN_DEBUG>)

set_target_properties(${PROJECT_NAME}
	PROPERTIES
	DEFINE_SYMBOL "NutshellEngineScriptManager_EXPORTS")

set(NTSHENGN_INSTALL_SCRIPTS_PATH "NtshEngn_InstallScripts_NOTFOUND" CACHE PATH "Give the option to install the built scripts into NutshellEngine\'s directory.")
if(EXISTS ${NTSHENGN_INSTALL_SCRIPTS_PATH})
	message(STATUS "Found NutshellEngine\'s modules directory at ${NTSHENGN_INSTALL_SCRIPTS_PATH}.")
	set(CMAKE_INSTALL_PREFIX ${NTSHENGN_INSTALL_SCRIPTS_PATH})
	if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
		install(TARGETS ${PROJECT_NAME}
			RUNTIME
			DESTINATION ${NTSHENGN_INSTALL_SCRIPTS_PATH})
	else()
		install(TARGETS ${PROJECT_NAME}
			LIBRARY
			DESTINATION ${NTSHENGN_INSTALL_SCRIPTS_PATH})
	endif()
elseif(NOT NTSHENGN_INSTALL_SCRIPTS_PATH STREQUAL "NtshEngn_InstallScripts_NOTFOUND")
	message(WARNING "Could not find NutshellEngine\'s directory at ${NTSHENGN_INSTALL_SCRIPTS_PATH}.")
endif()

add_custom_target(GenerateScriptManager
	COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_script_manager.py ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Generate ${SCRIPTS_DIRECTORY}/ntshengn_script_manager.cpp.")

add_custom_target(CopyAssets
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets
	COMMENT "Copy assets from ${CMAKE_SOURCE_DIR}/assets to project build folder ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets.")

add_dependencies(${PROJECT_NAME} GenerateScriptManager)
add_dependencies(${PROJECT_NAME} CopyAssets)