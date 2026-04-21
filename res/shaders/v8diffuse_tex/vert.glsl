#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_normal;

uniform mat4 uni_M;
uniform mat4 uni_VP;

out vec2 v_uv;        /* prosleđujemo UV u fragment */
out vec3 v_normal;    /* prosleđujemo normalu, ako želiš difuzno osvetljenje */
out vec3 v_colour;    /* ako želiš i boju verteksa, opcionalno */

void main()
{
    gl_Position = uni_VP * uni_M * vec4(in_position, 1.0);

    v_uv     = in_uv;
    v_normal = mat3(uni_M) * in_normal; /* transform normal, ili samo in_normal */
    v_colour = in_colour; /* ako želiš da iskoristiš boju verteksa */
}
