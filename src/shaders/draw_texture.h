// This file contains shaders for DrawTextureFancy, which let you
// draw textures in various ways: with gain and bias adjustments,
// with a colormap look-up and variable min/max ranges (e.g. for
// floating-point grayscale textures), select channels, etc.

#pragma once
#define SHADER(S) "#version 150\n" #S
const char *texture_shader_vs = SHADER(
in vec2 in_position;
uniform mat4 projection;
out vec2 texel;
void main()
{
    texel = in_position;
    gl_Position = projection*vec4(in_position, 0.0, 1.0);
}
);

const char *texture_shader_fs = SHADER(
in vec2 texel;
uniform int mode;
uniform vec4 gain;
uniform vec4 bias;
uniform vec4 selector;
uniform float blend;
uniform float range_min;
uniform float range_max;
uniform sampler2D channel0;
uniform sampler2D channel1;
out vec4 color;
void main()
{
    vec4 sample_rgba = gain*(texture(channel0, texel) + bias);
    vec4 sample_mono = texture(channel1, vec2((dot(sample_rgba,selector) - range_min) / (range_max - range_min), 0.0));
    color = mix(sample_rgba, sample_mono, blend);
}
);
#undef SHADER
