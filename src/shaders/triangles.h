const char *shader_triangles_vs =
    "#version 150\n"
    "in vec4 position;\n"
    "in vec2 texel;\n"
    "in vec4 color;\n"
    "uniform mat4 pvm;\n"
    "uniform vec2 ndc_offset;\n"
    "out vec4 vs_color;\n"
    "out vec2 vs_texel;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = pvm*position;\n"
    "    gl_Position.xy -= ndc_offset*gl_Position.w;\n"
    "    vs_color = color;\n"
    "    vs_texel = texel;\n"
    "}\n";

const char *shader_triangles_fs =
    "#version 150\n"
    "in vec4 vs_color;\n"
    "in vec2 vs_texel;\n"
    "uniform sampler2D sampler0;\n"
    "out vec4 color0;\n"
    "void main()\n"
    "{\n"
    "    color0 = vs_color*texture(sampler0, vs_texel);\n"
    "}\n";
