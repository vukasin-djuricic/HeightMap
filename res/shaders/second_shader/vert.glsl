#version 330

layout (location = 0) in vec3 position;

//layout (location = 1) in float scale; // <- prosledimo skalu teksture
layout (location = 2) in vec2 in_uv;

layout(location = 3) in vec3 in_normal;

layout(location = 4) in vec3 in_tangent;   // ako ste izračunali tangentu
layout(location = 5) in vec3 in_bitangent; // ako imate i bitangent

//out vec3 pass_colour;
out vec2 pass_uv;
out float pass_height;  // <- prosledimo visinu
out mat3  pass_TBN;

uniform mat4 uni_M;
uniform mat4 uni_VP;

void main()
{
	vec4 world_position = uni_M * vec4(position, 1.0);
	gl_Position = uni_VP * world_position;
	
	//pass_colour = colour;

	pass_height = world_position.y;

	pass_uv = in_uv;

    vec3 N  = normalize(mat3(uni_M) * in_normal);
    vec3 T  = normalize(mat3(uni_M) * in_tangent);
    vec3 B  = normalize(mat3(uni_M) * in_bitangent);

	
	pass_TBN = mat3(T, B, N);
	

}