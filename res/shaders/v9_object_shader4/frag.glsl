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
	
	float refraction_ratio = 1.00 / 1.52;
	vec3 refracted_view_vector = refract(view_vector, normalized_normal, refraction_ratio);
	
	float fresnel = dot(reflected_view_vector ,(view_vector*-1));
    fresnel = clamp(1.0 - fresnel, 0.0, 1.0);
    fresnel = pow(fresnel, 5.0);
	
	vec3 reflected_colour = texture(skybox_tex, reflected_view_vector).rgb;
	reflected_colour = mix(vec3(0.0), reflected_colour, 0.8);
	
	vec3 refracted_colour = texture(skybox_tex, refracted_view_vector).rgb;
	refracted_colour = mix(vec3(0.0), refracted_colour, 0.8);
	
	final_colour = vec4(mix(refracted_colour, reflected_colour, fresnel), 1.0);
	
}