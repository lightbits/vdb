//
// This shader can be used to render the points as colored meshes. This gives
// you MSAA for free (if enabled), but you end up with non-perfect circles.
// This option might be faster than the quad shader, especially for low
// vertex counts.
//
#pragma once
#define SHADER(S) "#version 150\n" #S
const char *shader_points_vs = SHADER(
in vec2 in_position;
in vec4 instance_position;
in vec2 instance_texel;
in vec4 instance_color;
uniform mat4 projection;
uniform mat4 model_to_view;
uniform vec2 point_size;
uniform int size_is_3D;
uniform sampler2D sampler0;
out vec4 vertex_color;
out vec2 quad_position;
void main()
{
    quad_position = in_position;
    vertex_color = instance_color*texture(sampler0, instance_texel);
    if (size_is_3D == 1)
    {
        vec4 position = model_to_view*instance_position;
        position.xy += point_size*in_position;
        gl_Position = projection*position;
    }
    else
    {
        vec4 position = model_to_view*instance_position;
        gl_Position = projection*position;
        float w = gl_Position.w;
        gl_Position.xy += point_size*in_position*w;
    }
}
);

const char *shader_points_fs = SHADER(
in vec4 vertex_color;
in vec2 quad_position;
out vec4 fragment_color;
void main()
{
    fragment_color = vertex_color;
}
);
#undef SHADER
