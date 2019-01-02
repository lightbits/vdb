#include "colormap_inferno.h"

GLenum TextureFormatToGL(vdbTextureFormat format)
{
    if (format == VDB_RGBA32F) return GL_RGBA32F;
    else if (format == VDB_RGBA8U) return GL_RGBA8;
    assert(false && "Unrecognized vdbTextureFormat");
    return GL_RGBA;
}

GLenum TextureFilterToGL(vdbTextureFilter filter)
{
    if (filter == VDB_NEAREST) return GL_NEAREST;
    else if (filter == VDB_LINEAR) return GL_LINEAR;
    else if (filter == VDB_LINEAR_MIPMAP) return GL_LINEAR_MIPMAP_LINEAR;
    assert(false && "Unrecognized vdbTextureFilter");
    return GL_NEAREST;
}

GLenum TextureWrapToGL(vdbTextureWrap wrap)
{
    if (wrap == VDB_CLAMP) return GL_CLAMP_TO_EDGE;
    else if (wrap == VDB_REPEAT) return GL_REPEAT;
    assert(false && "Unrecognized vdbTextureWrap");
    return GL_CLAMP_TO_EDGE;
}

void vdbSetTextureParameters(vdbTextureFilter filter, vdbTextureWrap wrap, bool generate_mipmaps)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (filter == VDB_LINEAR_MIPMAP) ? GL_LINEAR : TextureFilterToGL(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     TextureWrapToGL(wrap));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     TextureWrapToGL(wrap));
    if (filter == VDB_LINEAR_MIPMAP && generate_mipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);
}

GLuint TexImage2D(
    const void *data,
    int width,
    int height,
    GLenum data_format,
    GLenum data_type = GL_UNSIGNED_BYTE,
    GLenum mag_filter = GL_LINEAR,
    GLenum min_filter = GL_LINEAR,
    GLenum wrap_s = GL_CLAMP_TO_EDGE,
    GLenum wrap_t = GL_CLAMP_TO_EDGE,
    GLenum internal_format = GL_RGBA)
{
    GLuint result = 0;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 internal_format,
                 width,
                 height,
                 0,
                 data_format,
                 data_type,
                 data);
    if (min_filter == GL_LINEAR_MIPMAP_LINEAR)
        glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return result;
}

GLuint GetTexture(int slot)
{
    enum { MAX_TEXTURES = 1024 };
    static GLuint textures[MAX_TEXTURES] = {0};

    assert(slot >= 0 && slot < MAX_TEXTURES);

    if (!textures[slot])
    {
        glGenTextures(1, &textures[slot]);
        // todo: this?
        // glBindTexture(GL_TEXTURE_2D);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // glBindTexture(0);
    }

    return textures[slot];
}

void vdbLoadImage(int slot,
                  const void *data,
                  int width,
                  int height,
                  GLenum data_format,
                  GLenum data_type,
                  GLenum mag_filter = GL_LINEAR,
                  GLenum min_filter = GL_LINEAR,
                  GLenum wrap_s = GL_CLAMP_TO_EDGE,
                  GLenum wrap_t = GL_CLAMP_TO_EDGE,
                  GLenum internal_format = GL_RGBA)
{
    GLuint tex = GetTexture(slot);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    if (min_filter == GL_LINEAR_MIPMAP_LINEAR)
        glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 internal_format,
                 width,
                 height,
                 0,
                 data_format,
                 data_type,
                 data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void vdbLoadImageUint8(int slot, const void *data, int width, int height, int channels)
{
    assert(channels >= 1 && channels <= 4 && "'channels' must be 1,2,3 or 4");
    if      (channels == 1) vdbLoadImage(slot, data, width, height, GL_RED, GL_UNSIGNED_BYTE);
    else if (channels == 2) vdbLoadImage(slot, data, width, height, GL_RG, GL_UNSIGNED_BYTE);
    else if (channels == 3) vdbLoadImage(slot, data, width, height, GL_RGB, GL_UNSIGNED_BYTE);
    else if (channels == 4) vdbLoadImage(slot, data, width, height, GL_RGBA, GL_UNSIGNED_BYTE);
}

void vdbLoadImageFloat32(int slot, const void *data, int width, int height, int channels)
{
    assert(channels >= 1 && channels <= 4 && "'channels' must be 1,2,3 or 4");
    if      (channels == 1) vdbLoadImage(slot, data, width, height, GL_RED, GL_FLOAT);
    else if (channels == 2) vdbLoadImage(slot, data, width, height, GL_RG, GL_FLOAT);
    else if (channels == 3) vdbLoadImage(slot, data, width, height, GL_RGB, GL_FLOAT);
    else if (channels == 4) vdbLoadImage(slot, data, width, height, GL_RGBA, GL_FLOAT);
}

void vdbLoadImageFromFile(int slot, const char *filename, int *width, int *height, int *channels)
{
    int w,h,n;
    unsigned char *data = stbi_load(filename, &w, &h, &n, 4);
    assert(data && "Failed to load image from file.");
    vdbLoadImage(slot, data, w, h, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_RGBA);
    free(data);

    if (width) *width = w;
    if (height) *height = h;
    if (channels) *channels = n;
}

void vdbDrawImage(int slot, vdbTextureFilter filter, vdbTextureWrap wrap)
{
    glBindTexture(GL_TEXTURE_2D, GetTexture(slot));
    vdbSetTextureParameters(filter, wrap, false);
    vdbTriangles();
    #if VDB_FLIP_IMAGE_TEXEL_Y==1
    vdbColor(1,1,1,1); vdbTexel(0,1); vdbVertex(-1,-1);
    vdbColor(1,1,1,1); vdbTexel(1,1); vdbVertex(+1,-1);
    vdbColor(1,1,1,1); vdbTexel(1,0); vdbVertex(+1,+1);
    vdbColor(1,1,1,1); vdbTexel(1,0); vdbVertex(+1,+1);
    vdbColor(1,1,1,1); vdbTexel(0,0); vdbVertex(-1,+1);
    vdbColor(1,1,1,1); vdbTexel(0,1); vdbVertex(-1,-1);
    #else
    vdbColor(1,1,1,1); vdbTexel(0,0); vdbVertex(-1,-1);
    vdbColor(1,1,1,1); vdbTexel(1,0); vdbVertex(+1,-1);
    vdbColor(1,1,1,1); vdbTexel(1,1); vdbVertex(+1,+1);
    vdbColor(1,1,1,1); vdbTexel(1,1); vdbVertex(+1,+1);
    vdbColor(1,1,1,1); vdbTexel(0,1); vdbVertex(-1,+1);
    vdbColor(1,1,1,1); vdbTexel(0,0); vdbVertex(-1,-1);
    #endif
    vdbEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void vdbDrawImage(int slot, vdbTextureOptions options)
{
    static GLuint program = 0;
    static GLint attrib_position = 0;
    static GLint uniform_vmin = 0;
    static GLint uniform_vmax = 0;
    static GLint uniform_selector = 0;
    static GLint uniform_is_mono = 0;
    static GLint uniform_is_cmap = 0;
    static GLint uniform_sampler0 = 0;
    static GLint uniform_sampler1 = 0;
    static GLint uniform_pvm = 0;
    if (!program)
    {
        const char *vs =
            "#version 150\n"
            "in vec2 position;\n"
            "uniform mat4 pvm;\n"
            "out vec2 texel;\n"
            "void main()\n"
            "{\n"
            "    texel = vec2(0.5) + 0.5*position;\n"
            "    gl_Position = pvm*vec4(position, 0.0, 1.0);\n"
            "}\n";

        const char *fs =
            "#version 150\n"
            "in vec2 texel;\n"
            "uniform vec4 vmin;\n"
            "uniform vec4 vmax;\n"
            "uniform vec4 selector;\n"
            "uniform int is_mono;\n"
            "uniform int is_cmap;\n"
            "uniform sampler2D sampler0;\n"
            "uniform sampler2D sampler1;\n"
            "out vec4 color0;\n"
            "void main()\n"
            "{\n"
            "    color0 = texture(sampler0, texel);\n"
            "    color0 = (color0 - vmin) / (vmax - vmin);\n"
            "    color0 = clamp(color0, vec4(0.0), vec4(1.0));\n"
            "    if (is_mono == 1)\n"
            "    {\n"
            "        float i = clamp(dot(selector, color0), 0.0, 1.0);\n"
            "        if (is_cmap == 1)\n"
            "            color0 = texture(sampler1, vec2(i, 0.0));\n"
            "        else\n"
            "            color0 = vec4(i,i,i,1.0);\n"
            "    }\n"
            "}\n";

        program = LoadShaderFromMemory(vs, fs);
        attrib_position = glGetAttribLocation(program, "position");
        uniform_pvm = glGetUniformLocation(program, "pvm");
        uniform_sampler0 = glGetUniformLocation(program, "sampler0");
        uniform_sampler1 = glGetUniformLocation(program, "sampler1");
        uniform_vmin = glGetUniformLocation(program, "vmin");
        uniform_vmax = glGetUniformLocation(program, "vmax");
        uniform_selector = glGetUniformLocation(program, "selector");
        uniform_is_mono = glGetUniformLocation(program, "is_mono");
        uniform_is_cmap = glGetUniformLocation(program, "is_cmap");
    }
    assert(program);

    // upload 1D colormap as 2D texture of height 1
    static GLuint color_map_tex = TexImage2D(
        colormap_inferno,
        colormap_inferno_length,
        1,
        GL_RGB,
        GL_FLOAT,
        GL_LINEAR, GL_LINEAR,
        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_RGBA);

    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTexture(slot));
    vdbSetTextureParameters(options.filter, options.wrap, true);
    glUniform1i(uniform_sampler0, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, color_map_tex);
    glUniform1i(uniform_sampler1, 1);

    float pvm[4*4];
    vdbGetPVM(pvm);
    UniformMat4fv(uniform_pvm, 1, pvm);

    if (options.vmin.x == options.vmax.x &&
        options.vmin.y == options.vmax.y &&
        options.vmin.z == options.vmax.z &&
        options.vmin.w == options.vmax.w)
    {
        options.vmax.x = 1.0f;
        options.vmax.y = 1.0f;
        options.vmax.z = 1.0f;
        options.vmax.w = 1.0f;
    }

    if (options.selector.x == 0.0f &&
        options.selector.y == 0.0f &&
        options.selector.z == 0.0f &&
        options.selector.w == 0.0f)
    {
        assert(options.cmap == VDB_COLORMAP_NONE && "Colormaps only make sense if you combine the channels into a single scalar (using selector.xyzw as linear weights)");
        glUniform1i(uniform_is_mono, 0);
    }
    else
    {
        UniformVec4(uniform_selector, options.selector);
        glUniform1i(uniform_is_mono, 1);
    }

    UniformVec4(uniform_vmin, options.vmin);
    UniformVec4(uniform_vmax, options.vmax);
    glUniform1i(uniform_is_cmap, options.cmap == VDB_COLORMAP_NONE ? 0 : 1);

    {
        static GLuint vao = 0;
        static GLuint vbo = 0;
        if (!vao)
        {
            static float position[] = { -1,-1, 1,-1, 1,1, 1,1, -1,1, -1,-1 };
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
        }
        assert(vao);
        assert(vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(attrib_position);
        glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(attrib_position);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void vdbBindImage(int slot, vdbTextureFilter filter, vdbTextureWrap wrap)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTexture(slot));
    vdbSetTextureParameters(filter, wrap, false);
}

void vdbUnbindImage()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
