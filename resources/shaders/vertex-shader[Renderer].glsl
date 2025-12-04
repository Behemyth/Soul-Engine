#version 430 core

in vec4 vert_VS_in;

const vec2 offset = vec2(0.5, 0.5);

out vec2 textureCoord;

void main()
{
	textureCoord = vert_VS_in.xy*offset + offset; // scale vertex attribute to [0-1] range
	gl_Position = vert_VS_in;
}