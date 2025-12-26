##############################################
# Font Resource Loading

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			font_types.ixx
			font_face.ixx
			font_loader.ixx
)
