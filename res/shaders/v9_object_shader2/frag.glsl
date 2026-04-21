#version 330

in vec3 pass_normal;
in vec3 pass_world_position;

out vec4 final_colour;

uniform vec3 uni_object_colour;
uniform vec3 uni_light_colour;
uniform vec3 uni_light_direction;
uniform vec3 uni_ambient;
uniform vec3 uni_camera_position;

uniform samplerCube skybox_tex;

void main()
{

	vec3 view_vector = normalize(pass_world_position - uni_camera_position);
	vec3 normalized_normal = normalize(pass_normal);
	vec3 reflected_view_vector = reflect(view_vector, normalized_normal);
	
	vec3 fcol = texture(skybox_tex, reflected_view_vector).rgb;
	fcol = mix(vec3(0.0), fcol, 0.8);
	final_colour = vec4(fcol, 1.0);
	
}