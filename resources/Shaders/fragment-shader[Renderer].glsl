 #version 430

	layout( std430, binding=0 ) buffer texB 
	{
		vec4 tex[];
	};

	in vec2 textureCoord;

    out vec4 fragment;

	uniform uvec2 screen;


    void main(){

		uvec2 pos= uvec2(textureCoord.xy*vec2(screen));

		fragment= tex[pos.x+ screen.x*pos.y];

	}