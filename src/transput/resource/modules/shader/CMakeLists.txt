
##############################################
#Resource

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			shader.ixx
			shader_error.ixx
			shader_stage.ixx
			shader_manager.ixx
			modules/slang/slang_compiler.ixx
)
