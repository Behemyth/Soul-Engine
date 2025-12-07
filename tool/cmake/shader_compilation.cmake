# Shader compilation using Slang
# This module provides functions to compile Slang shaders to SPIR-V at build time
#
# Reflection is handled at runtime via SlangReflection in slang_reflection.ixx
# which uses Slang's actual reflection API for accurate type information.

# Find slangc compiler from Vulkan SDK
function(find_slang_compiler)
    if(NOT SLANGC_EXECUTABLE)
        if(DEFINED ENV{VULKAN_SDK})
            cmake_path(SET VULKAN_BIN_DIR NORMALIZE "$ENV{VULKAN_SDK}/Bin")
            find_program(SLANGC_EXECUTABLE
                NAMES slangc
                PATHS "${VULKAN_BIN_DIR}"
                NO_DEFAULT_PATH
            )
        endif()

        if(NOT SLANGC_EXECUTABLE)
            find_program(SLANGC_EXECUTABLE NAMES slangc)
        endif()

        if(SLANGC_EXECUTABLE)
            message(STATUS "Found slangc: ${SLANGC_EXECUTABLE}")
        else()
            message(WARNING "slangc not found - shader compilation will not be available")
        endif()
    endif()
endfunction()

# Internal: Compile a single shader entry point to SPIR-V
function(_compile_slang_entry_point)
    cmake_parse_arguments(
        SHADER
        "DEBUG"
        "SOURCE;OUTPUT;ENTRY_POINT;STAGE"
        "INCLUDE_DIRS;DEFINES"
        ${ARGN}
    )

    if(NOT SLANGC_EXECUTABLE)
        find_slang_compiler()
    endif()

    if(NOT SLANGC_EXECUTABLE)
        message(FATAL_ERROR "slangc is required for shader compilation")
    endif()

    # Normalize paths using cmake_path
    cmake_path(SET SHADER_SOURCE_PATH NORMALIZE "${SHADER_SOURCE}")
    cmake_path(SET SHADER_OUTPUT_PATH NORMALIZE "${SHADER_OUTPUT}")

    # Build slangc command
    set(SLANGC_ARGS
        "${SHADER_SOURCE_PATH}"
        -target spirv
        -entry "${SHADER_ENTRY_POINT}"
        -stage "${SHADER_STAGE}"
        -o "${SHADER_OUTPUT_PATH}"
    )

    # Add include directories
    foreach(include_dir ${SHADER_INCLUDE_DIRS})
        cmake_path(SET include_path NORMALIZE "${include_dir}")
        list(APPEND SLANGC_ARGS -I "${include_path}")
    endforeach()

    # Add defines
    foreach(define ${SHADER_DEFINES})
        list(APPEND SLANGC_ARGS -D "${define}")
    endforeach()

    # Add debug flag
    if(SHADER_DEBUG)
        list(APPEND SLANGC_ARGS -g)
    endif()

    # Create output directory if needed
    cmake_path(GET SHADER_OUTPUT_PATH PARENT_PATH OUTPUT_DIR)

    add_custom_command(
        OUTPUT "${SHADER_OUTPUT_PATH}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}"
        COMMAND ${SLANGC_EXECUTABLE} ${SLANGC_ARGS}
        DEPENDS "${SHADER_SOURCE_PATH}"
        COMMENT "Compiling shader: ${SHADER_ENTRY_POINT} (${SHADER_STAGE}) -> ${SHADER_OUTPUT}"
        VERBATIM
    )
endfunction()

# Map Slang [shader("...")] attribute to stage name
function(_slang_shader_type_to_stage SHADER_TYPE OUT_STAGE)
    if(SHADER_TYPE STREQUAL "vertex")
        set(${OUT_STAGE} "vertex" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "fragment" OR SHADER_TYPE STREQUAL "pixel")
        set(${OUT_STAGE} "fragment" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "compute")
        set(${OUT_STAGE} "compute" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "geometry")
        set(${OUT_STAGE} "geometry" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "hull")
        set(${OUT_STAGE} "hull" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "domain")
        set(${OUT_STAGE} "domain" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "mesh")
        set(${OUT_STAGE} "mesh" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "amplification" OR SHADER_TYPE STREQUAL "task")
        set(${OUT_STAGE} "amplification" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "raygeneration")
        set(${OUT_STAGE} "raygeneration" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "intersection")
        set(${OUT_STAGE} "intersection" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "anyhit")
        set(${OUT_STAGE} "anyhit" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "closesthit")
        set(${OUT_STAGE} "closesthit" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "miss")
        set(${OUT_STAGE} "miss" PARENT_SCOPE)
    elseif(SHADER_TYPE STREQUAL "callable")
        set(${OUT_STAGE} "callable" PARENT_SCOPE)
    else()
        set(${OUT_STAGE} "" PARENT_SCOPE)
    endif()
endfunction()

# Auto-compile all shaders in a directory
# Discovers .slang files, parses [shader("stage")] attributes, compiles each entry point
#
# Usage: compile_slang_shaders(
#   TARGET <target_name>
#   SOURCE_DIR <directory_with_slang_files>
#   OUTPUT_DIR <output_directory_for_spv>
#   [INCLUDE_DIRS <dir1> <dir2> ...]
#   [DEFINES <DEFINE1=VALUE> <DEFINE2> ...]
#   [DEBUG]
# )
#
# Output naming convention: <shader_name>.<stage>.spv
# Example: triangle.slang with vertex and fragment -> triangle.vertex.spv, triangle.fragment.spv
function(compile_slang_shaders)
    cmake_parse_arguments(
        SHADERS
        "DEBUG"
        "TARGET;SOURCE_DIR;OUTPUT_DIR"
        "INCLUDE_DIRS;DEFINES"
        ${ARGN}
    )

    if(NOT SLANGC_EXECUTABLE)
        find_slang_compiler()
    endif()

    # Normalize paths
    cmake_path(SET SOURCE_DIR_PATH NORMALIZE "${SHADERS_SOURCE_DIR}")
    cmake_path(SET OUTPUT_DIR_PATH NORMALIZE "${SHADERS_OUTPUT_DIR}")

    # Find all .slang files
    file(GLOB SLANG_FILES "${SOURCE_DIR_PATH}/*.slang")

    set(ALL_SHADER_OUTPUTS)

    foreach(SLANG_FILE ${SLANG_FILES})
        # Get shader name (filename without extension)
        cmake_path(GET SLANG_FILE STEM SHADER_NAME)

        # Read the file to find entry points
        file(READ "${SLANG_FILE}" SHADER_CONTENT)

        # Find all [shader("type")] annotations followed by function names
        # Pattern: [shader("type")] ... funcName(
        string(REGEX MATCHALL "\\[shader\\(\"([a-z]+)\"\\)\\][^a-zA-Z_]*([a-zA-Z_][a-zA-Z0-9_]*)[^(]*\\("
            SHADER_MATCHES "${SHADER_CONTENT}")

        foreach(MATCH ${SHADER_MATCHES})
            # Extract shader type and entry point name
            string(REGEX MATCH "\\[shader\\(\"([a-z]+)\"\\)\\]" TYPE_MATCH "${MATCH}")
            string(REGEX REPLACE "\\[shader\\(\"([a-z]+)\"\\)\\]" "\\1" SHADER_TYPE "${TYPE_MATCH}")

            # Extract function name (return type + name before parenthesis)
            string(REGEX MATCH "\\][^a-zA-Z_]*[a-zA-Z_][a-zA-Z0-9_<>:, \t\n]*[ \t\n]+([a-zA-Z_][a-zA-Z0-9_]*)[ \t\n]*\\("
                FUNC_MATCH "${MATCH}")
            string(REGEX REPLACE ".*[ \t\n]([a-zA-Z_][a-zA-Z0-9_]*)[ \t\n]*\\(" "\\1" ENTRY_POINT "${FUNC_MATCH}")

            # Map shader type to stage
            _slang_shader_type_to_stage("${SHADER_TYPE}" STAGE)

            if(STAGE AND ENTRY_POINT)
                # Output file: <name>.<stage>.spv
                cmake_path(SET OUTPUT_FILE NORMALIZE "${OUTPUT_DIR_PATH}/${SHADER_NAME}.${STAGE}.spv")

                # Compile this entry point
                if(SHADERS_DEBUG)
                    _compile_slang_entry_point(
                        SOURCE "${SLANG_FILE}"
                        OUTPUT "${OUTPUT_FILE}"
                        ENTRY_POINT "${ENTRY_POINT}"
                        STAGE "${STAGE}"
                        INCLUDE_DIRS ${SHADERS_INCLUDE_DIRS}
                        DEFINES ${SHADERS_DEFINES}
                        DEBUG
                    )
                else()
                    _compile_slang_entry_point(
                        SOURCE "${SLANG_FILE}"
                        OUTPUT "${OUTPUT_FILE}"
                        ENTRY_POINT "${ENTRY_POINT}"
                        STAGE "${STAGE}"
                        INCLUDE_DIRS ${SHADERS_INCLUDE_DIRS}
                        DEFINES ${SHADERS_DEFINES}
                    )
                endif()

                list(APPEND ALL_SHADER_OUTPUTS "${OUTPUT_FILE}")
                message(STATUS "  Shader: ${SHADER_NAME}.${STAGE} (entry: ${ENTRY_POINT})")
            endif()
        endforeach()
    endforeach()

    # Create target for all shaders
    if(ALL_SHADER_OUTPUTS)
        add_custom_target(${SHADERS_TARGET} ALL DEPENDS ${ALL_SHADER_OUTPUTS})
        message(STATUS "Shader target '${SHADERS_TARGET}' will compile ${CMAKE_MATCH_COUNT} shaders")
    else()
        # Create empty target if no shaders found
        add_custom_target(${SHADERS_TARGET})
        message(STATUS "No shaders found in ${SOURCE_DIR_PATH}")
    endif()
endfunction()
