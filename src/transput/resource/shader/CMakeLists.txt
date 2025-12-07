##############################################
# Shader module - backend-agnostic shader types and compiler interface

target_sources(soul-engine
    PUBLIC
        FILE_SET CXX_MODULES FILES
            shader.ixx
            shader_error.ixx
            shader_stage.ixx
            shader_types.ixx
            shader_compiler.ixx
)

# Add backend subdirectory
add_subdirectory(backend)
