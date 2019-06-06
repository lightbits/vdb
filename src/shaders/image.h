#pragma once
#define SHADER(S) "#version 150\n" #S
const char *shader_image_vs = SHADER(
in vec2 quad_pos;
uniform mat4 pvm;
uniform vec2 im_pos;
uniform vec2 im_size;
out vec2 texel;
void main()
{
    texel = quad_pos;
    gl_Position = pvm*vec4(im_pos + im_size*quad_pos, 0.0, 1.0);
}
);

const char *shader_image_fs = SHADER(
in vec2 texel;
uniform vec4 vmin;
uniform vec4 vmax;
uniform int is_mono;
uniform int is_cmap;
uniform sampler2D sampler0;
uniform sampler2D sampler1;
out vec4 color0;
void main()
{
    color0 = texture(sampler0, texel);
    color0 = (color0 - vmin) / (vmax - vmin);
    color0 = clamp(color0, vec4(0.0), vec4(1.0));
    if (is_mono == 1)
    {
        float i = color0.x;
        if (is_cmap == 1)
            color0 = texture(sampler1, vec2(i, 0.0));
        else
            color0 = vec4(i,i,i,1.0);
    }
}
);
#undef SHADER
