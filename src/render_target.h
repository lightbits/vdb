struct render_target_t
{
    GLuint fbo; // usage: glBindFramebuffer(GL_FRAMEBUFFER, fbo)
    GLuint *color; // usage: glBindTexture(GL_TEXTURE_2D, color[i])
    GLuint depth;
    int width, height; // usage: glViewport(0, 0, width, height)
    int num_color_attachments;
    GLint last_viewport[4];
};

namespace render_texture
{
    enum { max_render_textures = 1024 };
    static render_target_t render_textures[max_render_textures];
    static render_target_t *current_render_texture;

    static void EnableRenderTarget(render_target_t *rt)
    {
        glGetIntegerv(GL_VIEWPORT, rt->last_viewport);
        glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
        vdbViewporti(0, 0, rt->width, rt->height);
        current_render_texture = rt;
    }

    static void DisableRenderTarget(render_target_t *rt)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        vdbViewporti(rt->last_viewport[0], rt->last_viewport[1], rt->last_viewport[2], rt->last_viewport[3]);
        current_render_texture = NULL;
    }

    static void FreeRenderTarget(render_target_t *rt)
    {
        if (rt->fbo)   glDeleteFramebuffers(1, &rt->fbo);
        if (rt->depth) glDeleteRenderbuffers(1, &rt->depth);
        if (rt->color)
        {
            for (int i = 0; i < rt->num_color_attachments; i++)
                glDeleteTextures(1, &rt->color[i]);
            free(rt->color);
            rt->color = NULL;
        }
        rt->num_color_attachments = 0;
        rt->width = 0;
        rt->height = 0;
        rt->fbo = 0;
        rt->depth = 0;
    }

    static render_target_t MakeRenderTarget(int width, int height,
                                       GLenum mag_filter=GL_LINEAR,
                                       GLenum min_filter=GL_LINEAR,
                                       bool enable_depth=false,
                                       GLenum data_format=GL_RGBA,
                                       GLenum data_type=GL_UNSIGNED_BYTE,
                                       GLenum internal_color_format=GL_RGBA,
                                       int num_color_attachments = 1)
    {
        static GLint max_color_attachments = 0;
        if (!max_color_attachments)
            glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);
        assert(num_color_attachments <= max_color_attachments && "Number of requested color attachments exceeds device capabilities");

        render_target_t result = {0};
        result.width = width;
        result.height = height;
        result.num_color_attachments = num_color_attachments;

        glGenFramebuffers(1, &result.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, result.fbo);

        result.color = new GLuint[num_color_attachments];
        for (int i = 0; i < num_color_attachments; i++)
        {
            GLuint color = TexImage2D(NULL, width, height, data_format, data_type, mag_filter, min_filter, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, internal_color_format);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color, 0);
            result.color[i] = color;
        }

        if (enable_depth)
        {
            // DEPTH_COMPONENT16
            // DEPTH_COMPONENT24
            // DEPTH_COMPONENT32
            // DEPTH_COMPONENT32F
            result.depth = TexImage2D(NULL, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_DEPTH_COMPONENT24);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, result.depth, 0);
        }

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        assert(status == GL_FRAMEBUFFER_COMPLETE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return result;
    }
}

void vdbBeginRenderTarget(int slot, vdbRenderTargetSize size, vdbRenderTargetFormat format)
{
    using namespace render_texture;
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    assert(format.stencil_bits == 0 && "Stencil in RenderTarget is not implemented yet.");

    render_target_t *rt = render_textures + slot;
    bool should_create = false;
    if (rt->fbo)
    {
        if (rt->width != size.width || rt->height != size.height)
        {
            FreeRenderTarget(rt);
            should_create = true;
        }
    }
    else
    {
        should_create = true;
    }

    if (should_create)
    {
        GLenum data_format = GL_RGBA;
        GLenum data_type = GL_UNSIGNED_BYTE;
        GLenum internal_format = TextureFormatToGL(format.format);
        bool enable_depth = (format.depth_bits > 0);

        *rt = MakeRenderTarget(size.width, size.height, GL_LINEAR, GL_LINEAR, enable_depth, data_format, data_type, internal_format);
        EnableRenderTarget(rt);
        glClearColor(0,0,0,0);
        if (enable_depth) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        else glClear(GL_COLOR_BUFFER_BIT);
        DisableRenderTarget(rt);
    }

    EnableRenderTarget(rt);
}

void vdbEndRenderTarget(int slot)
{
    using namespace render_texture;
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    DisableRenderTarget(render_textures + slot);

    // todo: if LINEAR_MIPMAP, regenerate mipmaps
}

void vdbBindRenderTarget(int slot, vdbTextureFilter filter, vdbTextureWrap wrap)
{
    using namespace render_texture;
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    glActiveTexture(GL_TEXTURE0); // todo: let user specify this
    glBindTexture(GL_TEXTURE_2D, render_textures[slot].color[0]);
    vdbSetTextureParameters(filter, wrap);
}

void vdbUnbindRenderTarget()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawRenderTargetWithDepth(render_target_t rt)
{
    #define SHADER(S) "#version 150\n" #S
    const char *vs = SHADER(
        in vec2 position;
        out vec2 texel;
        void main()
        {
            texel = vec2(0.5) + 0.5*position;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    );

    const char *fs = SHADER(
        in vec2 texel;
        uniform sampler2D sampler0;
        uniform sampler2D sampler1;
        out vec4 out_color;
        void main()
        {
            out_color = texture(sampler0, texel);
            gl_FragDepth = texture(sampler1, texel).x;
        }
    );
    #undef SHADER

    static GLuint program = LoadShaderFromMemory(vs,fs);
    static GLint attrib_position = glGetAttribLocation(program, "position");
    static GLint uniform_sampler0 = glGetUniformLocation(program, "sampler0");
    static GLint uniform_sampler1 = glGetUniformLocation(program, "sampler1");

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
    glUseProgram(program);
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(uniform_sampler1, 1);
    glBindTexture(GL_TEXTURE_2D, rt.depth);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uniform_sampler0, 0);
    glBindTexture(GL_TEXTURE_2D, rt.color[0]);
    glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(attrib_position);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(attrib_position);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void vdbDrawRenderTarget(int slot, vdbTextureFilter filter, vdbTextureWrap wrap)
{
    using namespace render_texture;
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_textures[slot].color[0]);
    vdbSetTextureParameters(filter, wrap);
    vdbBeginTriangles();
    vdbColor(1,1,1,1);
    vdbTexel(0,0); vdbVertex(-1,-1);
    vdbTexel(1,0); vdbVertex(+1,-1);
    vdbTexel(1,1); vdbVertex(+1,+1);
    vdbTexel(1,1); vdbVertex(+1,+1);
    vdbTexel(0,1); vdbVertex(-1,+1);
    vdbTexel(0,0); vdbVertex(-1,-1);
    vdbEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}
