//
// This shader can be used to render the points as colored meshes. This gives
// you MSAA for free (if enabled), but you end up with non-perfect circles.
// This option might be faster than the quad shader, especially for low
// vertex counts.
//
#pragma once
#define SHADER(S) "#version 150\n" #S
const char *point_shader_vs = SHADER(
in vec2 in_position;
in vec3 instance_position;
in vec4 instance_color;
uniform mat4 projection;
uniform mat4 model_to_view;
uniform float point_size;
// uniform float reflection; // I used this once to spin the point around every frame
                             // to make it look like a circle
out vec4 vertex_color;
out vec2 quad_position;
void main()
{
    quad_position = in_position;
    vertex_color = instance_color;
    vec4 position = model_to_view*vec4(instance_position,1.0) + point_size*vec4(in_position,0.0,0.0);
    // vec4 position = model_to_view*vec4(instance_position,1.0) + reflection*point_size*vec4(in_position,0.0,0.0);
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
}
);
#undef SHADER
