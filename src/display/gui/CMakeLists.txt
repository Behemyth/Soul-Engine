
##############################################
#GUI

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			ui_types.ixx
			ui_state.ixx
			draw_list.ixx
			widget.ixx
			layout.ixx
			font_atlas.ixx
			ui_renderer.ixx
			gui_module.ixx
)

add_subdirectory(widgets)
add_subdirectory(backend)
