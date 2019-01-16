#pragma once
#define SHADER(S) "#version 150\n" #S
const char *shader_thick_lines_vs = SHADER(
in vec2 in_position;
in vec4 instance_position0;
in vec2 instance_texel0;
in vec4 instance_color0;
in vec4 instance_position1;
in vec2 instance_texel1;
in vec4 instance_color1;
uniform mat4 projection;
uniform mat4 model_to_view;
uniform vec2 line_width;
uniform float aspect;
uniform vec2 ndc_offset;
uniform int width_is_3D;
uniform sampler2D sampler0;
out vec4 vertex_color;
void main()
{
    {
        mat4 pvm = projection*model_to_view;
        vec4 clip0 = pvm*instance_position0;
        vec4 clip1 = pvm*instance_position1;
        vec2 tangent = (clip1.xy/clip1.w) - (clip0.xy/clip0.w);
        tangent.x *= aspect;
        vec2 normal = vec2(-tangent.y, tangent.x);
        normal = normalize(normal);

        // todo: normal not in right direction...

        if (in_position.x < 0.0)
        {
            gl_Position = clip0;
            gl_Position.xy += line_width*in_position.y*normal*clip0.w;
            vertex_color = instance_color0*texture(sampler0, instance_texel0);
        }
        else
        {
            gl_Position = clip1;
            gl_Position.xy += line_width*in_position.y*normal*clip1.w;
            vertex_color = instance_color1*texture(sampler0, instance_texel1);
        }
    }
    gl_Position.xy -= ndc_offset*gl_Position.w;
}
);

const char *shader_thick_lines_fs = SHADER(
in vec4 vertex_color;
out vec4 fragment_color;
void main()
{
    fragment_color = vertex_color;
}
);
#undef SHADER
