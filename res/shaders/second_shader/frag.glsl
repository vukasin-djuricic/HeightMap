#version 330

//in vec3 pass_colour;
in vec2 pass_uv;
in float pass_height;

in mat3  pass_TBN;

out vec4 final_colour;

uniform sampler2D rock; // Rock tekstura
uniform sampler2D lava; // Lava tekstura
uniform sampler2D rockNormalMap; // Rock tekstura
uniform sampler2D lavaNormalMap; // Lava tekstura
uniform float scale;

//const float minHeight = -0.8;
//const float maxHeight = -0.6;

const float minHeight = 0.5;
const float maxHeight = 1.5;

void main()
{
    float scale_1 = 0.0;
    if(scale == 100)
    {
        scale_1 = 100;
    }
    else
    {scale_1 = 5;}

    float threshold = 2.0;  // recimo da je 2.0 Y visina

	vec4 lavaColor = texture(lava, pass_uv*scale_1);
    vec4 rockColor = texture(rock, pass_uv*scale_1);

	float t = smoothstep(minHeight, maxHeight, pass_height);
 	// t=0 = lava, t=1 = rock

	vec4 color = mix(lavaColor, rockColor, t);

	vec3 lavaNormal = texture(lavaNormalMap, pass_uv*scale_1).xyz * 2.0 - 1.0;
    vec3 rockNormal = texture(rockNormalMap, pass_uv*scale_1).xyz * 2.0 - 1.0;

    //na slican nacin se mixuju
    vec3 blendedNormalTangent = normalize(mix(lavaNormal, rockNormal, t));

	//oblik mat3(t b n)
    vec3 blendedNormalWorld = normalize(pass_TBN * blendedNormalTangent);

	// najjednostavniji lambert
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5)); // pr
    float diffuseFactor = max(dot(blendedNormalWorld, lightDir), 0.0);

	
    vec3 final = color.rgb * diffuseFactor;


	final_colour = vec4(final, 1.0);




   // final_colour = mixedColor - sada je samo "color";





    /*if(pass_height < threshold)
    {
        // Niži delovi -> lava
        final_colour = texture(lava, pass_uv*100);
    }
    else
    {
        // Viši delovi -> rock
        final_colour = texture(rock, pass_uv*100);
    }*/
}
