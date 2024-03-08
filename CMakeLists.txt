cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
set(NAME NutshellEngine-Editor)
project(${NAME} VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
add_subdirectory(external/nml)

set(EXTERNAL_HEADERS
	external/nlohmann/json.hpp)

set(COMMON_COMPONENTS_HEADERS
	src/common/components/camera.h
	src/common/components/collidable.h
	src/common/components/component.h
	src/common/components/light.h
	src/common/components/renderable.h
	src/common/components/rigidbody.h
	src/common/components/scriptable.h
	src/common/components/transform.h)

set(COMMON_HEADERS
	${COMMON_COMPONENTS_HEADERS}
	src/common/common.h
	src/common/entity.h
	src/common/signal_emitter.h)

set(RENDERER_SOURCES
	src/renderer/renderer_resource_manager.cpp)
set(RENDERER_HEADERS
	src/renderer/renderer_resource_manager.h)

set(UNDO_COMMANDS_SOURCES
	src/undo_commands/add_entity_component_command.cpp
	src/undo_commands/change_entity_component_command.cpp
	src/undo_commands/change_entity_name_command.cpp
	src/undo_commands/change_entity_persistence_command.cpp
	src/undo_commands/clear_scene_command.cpp
	src/undo_commands/copy_entity_command.cpp
	src/undo_commands/create_entity_command.cpp
	src/undo_commands/destroy_entity_command.cpp
	src/undo_commands/open_scene_command.cpp
	src/undo_commands/remove_entity_component_command.cpp)
set(UNDO_COMMANDS_HEADERS
	src/undo_commands/add_entity_component_command.h
	src/undo_commands/change_entity_component_command.h
	src/undo_commands/change_entity_name_command.h
	src/undo_commands/change_entity_persistence_command.h
	src/undo_commands/clear_scene_command.h
	src/undo_commands/copy_entity_command.h
	src/undo_commands/create_entity_command.h
	src/undo_commands/destroy_entity_command.h
	src/undo_commands/open_scene_command.h
	src/undo_commands/remove_entity_component_command.h)

set(WIDGET_SOURCES
	src/widgets/boolean_widget.cpp
	src/widgets/color_picker_widget.cpp
	src/widgets/combo_box_widget.cpp
	src/widgets/edit_menu.cpp
	src/widgets/file_menu.cpp
	src/widgets/file_selector_widget.cpp
	src/widgets/main_window.cpp
	src/widgets/new_project_directory_path_widget.cpp
	src/widgets/new_project_name_widget.cpp
	src/widgets/new_project_widget.cpp
	src/widgets/open_project_widget.cpp
	src/widgets/project_window.cpp
	src/widgets/scalar_widget.cpp
	src/widgets/separator_line.cpp
	src/widgets/vector2_widget.cpp
	src/widgets/vector3_widget.cpp
	src/widgets/view_menu.cpp)
set(WIDGET_HEADERS
	src/widgets/boolean_widget.h
	src/widgets/color_picker_widget.h
	src/widgets/combo_box_widget.h
	src/widgets/edit_menu.h
	src/widgets/file_menu.h
	src/widgets/file_selector_widget.h
	src/widgets/main_window.h
	src/widgets/new_project_directory_path_widget.h
	src/widgets/new_project_name_widget.h
	src/widgets/new_project_widget.h
	src/widgets/open_project_widget.h
	src/widgets/project_window.h
	src/widgets/scalar_widget.h
	src/widgets/separator_line.h
	src/widgets/vector2_widget.h
	src/widgets/vector3_widget.h
	src/widgets/view_menu.h)

set(SOURCES
	${RENDERER_SOURCES}
	${UNDO_COMMANDS_SOURCES}
	${WIDGET_SOURCES})
set(HEADERS
	${EXTERNAL_HEADERS}
	${COMMON_HEADERS}
	${RENDERER_HEADERS}
	${UNDO_COMMANDS_HEADERS}
	${WIDGET_HEADERS})

add_executable(${PROJECT_NAME} main.cpp ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Qt6::Core
		Qt6::Gui
		Qt6::Widgets
		nml)

target_include_directories(${PROJECT_NAME}
	PRIVATE
		external/nml)

target_compile_options(${PROJECT_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>)

target_compile_definitions(${PROJECT_NAME}
	PUBLIC
		$<$<CONFIG:Debug>:NTSHENGN_DEBUG>
		$<$<CONFIG:MinSizeRel>:NTSHENGN_RELEASE>
		$<$<CONFIG:Release>:NTSHENGN_RELEASE>
		$<$<CONFIG:RelWithDebInfo>:NTSHENGN_DEBUG>)

add_custom_target(CopyAssets
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets
	COMMENT "Copy assets from ${CMAKE_SOURCE_DIR}/assets to project build folder ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets.")

add_dependencies(${PROJECT_NAME} CopyAssets)