//
// This shader can be used to render the points as colored meshes. This gives
// you MSAA for free (if enabled), but you end up with non-perfect circles.
// This option might be faster than the quad shader, especially for low
// vertex counts.
//
#pragma once
#define SHADER(S) "#version 150\n" #S
const char *immediate_point_shader_vs = SHADER(
in vec2 in_position;
in vec4 instance_position;
in vec2 instance_texel;
in vec4 instance_color;
uniform mat4 projection;
uniform mat4 model_to_view;
uniform float point_size;
uniform int size_is_3D;
uniform vec2 resolution;
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
        gl_Position = projection*(position + point_size*vec4(in_position, 0.0, 0.0));
    }
    else
    {
        vec4 position = model_to_view*instance_position;
        gl_Position = projection*position;
        float w = gl_Position.w;
        vec2 point_size_ndc = 2.0*point_size/resolution;
        gl_Position.xy += point_size_ndc*in_position*w;
    }
}
);

const char *immediate_point_shader_fs = SHADER(
in vec4 vertex_color;
in vec2 quad_position;
out vec4 fragment_color;
void main()
{
    fragment_color = vertex_color;
}
);
#undef SHADER
