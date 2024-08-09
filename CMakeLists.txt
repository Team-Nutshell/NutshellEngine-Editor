cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
set(NAME NutshellEngine-Editor)
project(${NAME} VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets REQUIRED)
add_subdirectory(external/nml)

set(NTSHENGN_EDITOR_DEFINES "")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND NTSHENGN_EDITOR_DEFINES NTSHENGN_COMPILER_GCC)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND NTSHENGN_EDITOR_DEFINES NTSHENGN_COMPILER_CLANG)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND NTSHENGN_EDITOR_DEFINES NTSHENGN_COMPILER_MSVC)
else()
    message(WARNING "Using an untested compiler '${CMAKE_CXX_COMPILER_ID}'. "
        "Consider using GCC, Clang or MSVC.")
    list(APPEND NTSHENGN_EDITOR_DEFINES NTSHENGN_COMPILER_UNKNOWN)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND NTSHENGN_EDITOR_DEFINES NTSHENGN_OS_WINDOWS _CRT_SECURE_NO_WARNINGS NOMINMAX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND NTSHENGN_EDITOR_DEFINES NTSHENGN_OS_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND NTSHENGN_EDITOR_DEFINES NTSHENGN_OS_MACOS)
endif()

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
	src/common/bounding_volumes.h
	src/common/editor_parameters.h
	src/common/entity.h
	src/common/global_info.h
	src/common/logger.h
	src/common/save_title_changer.h
	src/common/scene_manager.h
	src/common/signal_emitter.h)

set(COMMON_SOURCES
	src/common/editor_parameters.cpp
	src/common/logger.cpp
	src/common/scene_manager.cpp)

set(RENDERER_SOURCES
	src/renderer/collider_mesh.cpp
	src/renderer/renderer.cpp
	src/renderer/renderer_resource_manager.cpp)
set(RENDERER_HEADERS
	src/renderer/collider_mesh.h
	src/renderer/renderer.h
	src/renderer/renderer_camera.h
	src/renderer/renderer_resources.h
	src/renderer/renderer_resource_manager.h)

set(UNDO_COMMANDS_SOURCES
	src/undo_commands/add_entity_component_command.cpp
	src/undo_commands/change_entities_component_command.cpp
	src/undo_commands/change_entity_name_command.cpp
	src/undo_commands/change_entity_persistence_command.cpp
	src/undo_commands/clear_scene_command.cpp
	src/undo_commands/copy_entity_command.cpp
	src/undo_commands/create_entities_from_model_command.cpp
	src/undo_commands/create_entity_command.cpp
	src/undo_commands/destroy_entities_command.cpp
	src/undo_commands/open_scene_command.cpp
	src/undo_commands/remove_entity_component_command.cpp)
set(UNDO_COMMANDS_HEADERS
	src/undo_commands/add_entity_component_command.h
	src/undo_commands/change_entities_component_command.h
	src/undo_commands/change_entity_name_command.h
	src/undo_commands/change_entity_persistence_command.h
	src/undo_commands/clear_scene_command.h
	src/undo_commands/copy_entity_command.h
	src/undo_commands/create_entities_from_model_command.h
	src/undo_commands/create_entity_command.h
	src/undo_commands/destroy_entities_command.h
	src/undo_commands/open_scene_command.h
	src/undo_commands/remove_entity_component_command.h)

set(WIDGET_SOURCES
	src/widgets/add_component_button.cpp
	src/widgets/add_component_menu.cpp
	src/widgets/asset_list.cpp
	src/widgets/build_bar.cpp
	src/widgets/boolean_widget.cpp
	src/widgets/camera_component_widget.cpp
	src/widgets/color_picker_widget.cpp
	src/widgets/collidable_component_widget.cpp
	src/widgets/combo_box_widget.cpp
	src/widgets/component_list.cpp
	src/widgets/component_scroll_area.cpp
	src/widgets/component_title_widget.cpp
	src/widgets/edit_menu.cpp
	src/widgets/editor_parameters_widget.cpp
	src/widgets/entity_info_name_widget.cpp
	src/widgets/entity_info_panel.cpp
	src/widgets/entity_info_persistence_widget.cpp
	src/widgets/entity_list.cpp
	src/widgets/entity_list_item.cpp
	src/widgets/entity_list_menu.cpp
	src/widgets/entity_panel.cpp
	src/widgets/file_menu.cpp
	src/widgets/file_push_button.cpp
	src/widgets/file_selector_widget.cpp
	src/widgets/image_viewer.cpp
	src/widgets/integer_widget.cpp
	src/widgets/key_select_widget.cpp
	src/widgets/light_component_widget.cpp
	src/widgets/log_bar.cpp
	src/widgets/log_bar_menu.cpp
	src/widgets/logs_widget.cpp
	src/widgets/main_window.cpp
	src/widgets/material_ntml_file_widget.cpp
	src/widgets/model_ntmd_file_widget.cpp
	src/widgets/model_ntmd_primitive_widget.cpp
	src/widgets/new_project_directory_path_widget.cpp
	src/widgets/new_project_name_widget.cpp
	src/widgets/new_project_widget.cpp
	src/widgets/new_scene_message_box.cpp
	src/widgets/new_script_message_box.cpp
	src/widgets/open_project_widget.cpp
	src/widgets/options_ntop_file_widget.cpp
	src/widgets/options_menu.cpp
	src/widgets/project_window.cpp
	src/widgets/renderable_component_widget.cpp
	src/widgets/resource_panel.cpp
	src/widgets/rigidbody_component_widget.cpp
	src/widgets/sampler_ntsp_file_widget.cpp
	src/widgets/scalar_widget.cpp
	src/widgets/scriptable_component_widget.cpp
	src/widgets/separator_line.cpp
	src/widgets/string_widget.cpp
	src/widgets/transform_component_widget.cpp
	src/widgets/vector2_widget.cpp
	src/widgets/vector3_widget.cpp
	src/widgets/view_menu.cpp)
set(WIDGET_HEADERS
	src/widgets/add_component_button.h
	src/widgets/add_component_menu.h
	src/widgets/asset_list.h
	src/widgets/build_bar.h
	src/widgets/boolean_widget.h
	src/widgets/camera_component_widget.h
	src/widgets/color_picker_widget.h
	src/widgets/collidable_component_widget.h
	src/widgets/combo_box_widget.h
	src/widgets/component_list.h
	src/widgets/component_scroll_area.h
	src/widgets/component_title_widget.h
	src/widgets/edit_menu.h
	src/widgets/editor_parameters_widget.h
	src/widgets/entity_info_name_widget.h
	src/widgets/entity_info_panel.h
	src/widgets/entity_info_persistence_widget.h
	src/widgets/entity_list.h
	src/widgets/entity_list_item.h
	src/widgets/entity_list_menu.h
	src/widgets/entity_panel.h
	src/widgets/file_menu.h
	src/widgets/file_push_button.h
	src/widgets/file_selector_widget.h
	src/widgets/image_viewer.h
	src/widgets/integer_widget.h
	src/widgets/key_select_widget.h
	src/widgets/light_component_widget.h
	src/widgets/log_bar.h
	src/widgets/log_bar_menu.h
	src/widgets/logs_widget.h
	src/widgets/main_window.h
	src/widgets/material_ntml_file_widget.h
	src/widgets/model_ntmd_file_widget.h
	src/widgets/model_ntmd_primitive_widget.h
	src/widgets/new_project_directory_path_widget.h
	src/widgets/new_project_name_widget.h
	src/widgets/new_project_widget.h
	src/widgets/new_scene_message_box.h
	src/widgets/new_script_message_box.h
	src/widgets/open_project_widget.h
	src/widgets/options_ntop_file_widget.h
	src/widgets/options_menu.h
	src/widgets/project_window.h
	src/widgets/renderable_component_widget.h
	src/widgets/resource_panel.h
	src/widgets/rigidbody_component_widget.h
	src/widgets/sampler_ntsp_file_widget.h
	src/widgets/scalar_widget.h
	src/widgets/scriptable_component_widget.h
	src/widgets/separator_line.h
	src/widgets/string_widget.h
	src/widgets/transform_component_widget.h
	src/widgets/vector2_widget.h
	src/widgets/vector3_widget.h
	src/widgets/view_menu.h)

set(SOURCES
	${COMMON_SOURCES}
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
		Qt6::OpenGL
		Qt6::OpenGLWidgets
		nml)

target_include_directories(${PROJECT_NAME}
	PRIVATE
		external/cgltf
		external/nml
		external/stb)

target_compile_options(${PROJECT_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>)

target_compile_definitions(${PROJECT_NAME}
	PUBLIC
		${NTSHENGN_EDITOR_DEFINES}
		$<$<CONFIG:Debug>:NTSHENGN_DEBUG>
		$<$<CONFIG:MinSizeRel>:NTSHENGN_RELEASE>
		$<$<CONFIG:Release>:NTSHENGN_RELEASE>
		$<$<CONFIG:RelWithDebInfo>:NTSHENGN_DEBUG>)

add_custom_target(CopyAssets
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets
	COMMENT "Copy assets from ${CMAKE_SOURCE_DIR}/assets to project build folder ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets.")

add_dependencies(${PROJECT_NAME} CopyAssets)
