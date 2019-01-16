//
// This shader can be used to render the points as pixel-perfect circles,
// using textured quads. This option might be slower than the vertex version
// especially if you restrict yourself to low vertex count.
//
#pragma once
#define SHADER(S) "#version 150\n" #S
const char *point_shader_vs = SHADER(
in vec2 in_position;
in vec4 in_color;
in vec3 instance_position;
in vec4 instance_color;
uniform mat4 projection;
uniform mat4 model_to_view;
uniform float point_size;
out vec4 vertex_color;
out vec2 quad_position;
void main()
{
    quad_position = in_position;
    vertex_color = instance_color*in_color;
    vec4 position = model_to_view*vec4(instance_position,1.0) + point_size*vec4(in_position,0.0,0.0);
    gl_Position = projection*position;
}
);

const char *point_shader_fs = SHADER(
in vec4 vertex_color;
in vec2 quad_position;
out vec4 fragment_color;
void main()
{
    fragment_color = vertex_color;

    // NO ALPHA BLENDING
    if (dot(quad_position,quad_position) > 1.0)
        discard;
}
);
#undef SHADER
