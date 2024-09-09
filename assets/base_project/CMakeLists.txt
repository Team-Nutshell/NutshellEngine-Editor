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

set(NTSHENGN_BUILD_IN_EDITOR OFF CACHE BOOL "Whether this CMake is executed in NutshellEngine's Editor or not.")
if(NOT NTSHENGN_BUILD_IN_EDITOR)
	message(STATUS "Generate ntshengn_script_manager.cpp with CMake.")
	find_package(Python3 REQUIRED COMPONENTS Interpreter)
	execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/script_manager/generate_script_manager.py ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(SCRIPTS_DIRECTORY "scripts")
file(GLOB_RECURSE SCRIPTS "${SCRIPTS_DIRECTORY}/*.h")
message(STATUS ${SCRIPTS})

set(SCRIPT_MANAGER
	${SCRIPTS_DIRECTORY}/script_manager/ntshengn_script_manager.h
	${SCRIPTS_DIRECTORY}/script_manager/ntshengn_script_manager.cpp)

add_library(${PROJECT_NAME} SHARED ${SCRIPT_MANAGER} ${SCRIPTS})

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		NutshellEngine::Common
		${NTSHENGN_LIBRARIES})

target_compile_options(${PROJECT_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>)

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
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
		DESTINATION ${NTSHENGN_INSTALL_SCRIPTS_PATH})
elseif(NOT NTSHENGN_INSTALL_SCRIPTS_PATH STREQUAL "NtshEngn_InstallScripts_NOTFOUND")
	message(WARNING "Could not find NutshellEngine\'s directory at ${NTSHENGN_INSTALL_SCRIPTS_PATH}.")
endif()

if(NOT NTSHENGN_BUILD_IN_EDITOR)
	add_custom_target(GenerateScriptManager
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPTS_DIRECTORY}/script_manager/generate_script_manager.py ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generate ntshengn_script_manager.cpp during build.")
endif()

add_custom_target(CopyAssets
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets
	COMMENT "Copy assets from ${CMAKE_SOURCE_DIR}/assets to project build folder ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets.")

if(NOT NTSHENGN_BUILD_IN_EDITOR)
	add_dependencies(${PROJECT_NAME} GenerateScriptManager)
endif()
add_dependencies(${PROJECT_NAME} CopyAssets)

set(NTSHENGN_EXECUTABLE_PATH "NtshEngn_Executable_NOTFOUND" CACHE FILEPATH "Path to NutshellEngine\'s executable to launch the application.")
if(EXISTS ${NTSHENGN_EXECUTABLE_PATH} AND NOT IS_DIRECTORY ${NTSHENGN_EXECUTABLE_PATH})
	message(STATUS "Found NutshellEngine\'s executable at ${NTSHENGN_EXECUTABLE_PATH}")
	get_filename_component(NTSHENGN_EXECUTABLE_DIRECTORY ${NTSHENGN_EXECUTABLE_PATH} DIRECTORY)
	add_custom_target(ExecuteApplication
		COMMAND ${NTSHENGN_EXECUTABLE_PATH}
		WORKING_DIRECTORY ${NTSHENGN_EXECUTABLE_DIRECTORY}
		COMMENT "Executing the application.")
elseif(NOT NTSHENGN_EXECUTABLE_PATH STREQUAL "NtshEngn_Executable_NOTFOUND" OR IS_DIRECTORY ${NTSHENGN_EXECUTABLE_PATH})
	message(WARNING "NutshellEngine\'s executable could not be found at ${NTSHENGN_EXECUTABLE_PATH} or path is not a file.")
endif()