/**
 * @file shader.ixx
 * @brief Shader module public interface
 * 
 * This module provides:
 * - Backend-agnostic shader types (ShaderReflectionData, etc.)
 * - Abstract compiler interface (IShaderCompiler)
 * - Compilation options and error handling
 * 
 * Backend implementations (Slang, DXC, etc.) are in separate submodules.
 */
export module synodic.soul.transput:shader;

export import :shader_error;
export import :shader_stage;
export import :shader_types;
export import :shader_compiler;
