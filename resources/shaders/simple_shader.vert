#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;


void main() {
    gl_Position = vec4(inPosition.xy, 0.0, 1.0);
    fragColor = vec3(0,0.7,1);
}