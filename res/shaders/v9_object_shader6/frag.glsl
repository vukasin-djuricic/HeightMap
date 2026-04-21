#version 330

in vec3 pass_normal;
in vec3 pass_world_position;

out vec4 final_colour;

uniform vec3 uni_object_colour;
uniform vec3 uni_light_colour;
uniform vec3 uni_light_direction;
uniform vec3 uni_ambient;
uniform vec3 uni_camera_position;


void main()
{
	vec3 unit_light_vector = normalize(-uni_light_direction);
	float ambient = 0.2;

    vec3 normalized_normal = normalize(pass_normal);
    
	float light_factor = clamp(dot(normalized_normal, unit_light_vector), 0, 1);

    vec3 view_vector = normalize(pass_world_position - uni_camera_position);
    float spec_factor = dot(reflect(view_vector, normalized_normal), unit_light_vector);
    spec_factor = clamp(spec_factor, 0, 1);
    spec_factor = pow(spec_factor, 10.0);
	
	float total_light = light_factor + spec_factor;
	
	float cell_steps = 3.0;
	total_light = ambient + (floor(total_light * cell_steps) / cell_steps) * (1.0 - ambient);
	//	total_light = ambient + ((total_light / cell_steps) * cell_steps) * (1.0 - ambient);

	float silhouette = dot(view_vector * -1, normalized_normal);
	if(abs(silhouette) < 0.2) total_light = 0.0;
	
	final_colour = vec4(total_light, total_light, total_light, 1.0);
	
}