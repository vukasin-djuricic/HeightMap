#version 330 core

in vec2 v_uv;
in vec3 v_normal;
in vec3 v_colour;

/* Uniformi sa CPU strane */
uniform sampler2D tex0;
uniform vec3 uni_object_colour;
uniform vec3 uni_light_colour;
uniform vec3 uni_light_direction;
uniform vec3 uni_ambient;

out vec4 out_color;

void main()
{
    /* Uzmi boju iz teksture */
    vec4 tex_color = texture(tex0, v_uv);

    /* Normalizuj normalu i direction (svetlo), pa difuzni dot */
    vec3 N = normalize(v_normal);
    vec3 L = normalize(-uni_light_direction);
    float diff = max(dot(N, L), 0.0);

    /* Kombinacija boja (tekstura * object_colour) i osvetljenje (ambient + diff * light) */
    vec3 base     = tex_color.rgb * uni_object_colour;
    vec3 lighting = uni_ambient + diff * uni_light_colour;

    /* Konačna boja */
    vec3 final_color = base * lighting;
    out_color        = vec4(final_color, tex_color.a);
}
