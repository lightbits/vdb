struct render_texture_t
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
    static render_texture_t render_textures[max_render_textures];
    static render_texture_t *current_render_texture;

    static void EnableRenderTexture(render_texture_t *rt)
    {
        glGetIntegerv(GL_VIEWPORT, rt->last_viewport);
        glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
        vdbViewporti(0, 0, rt->width, rt->height);
        current_render_texture = rt;
    }

    static void DisableRenderTexture(render_texture_t *rt)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        vdbViewporti(rt->last_viewport[0], rt->last_viewport[1], rt->last_viewport[2], rt->last_viewport[3]);
        current_render_texture = NULL;
    }

    static void FreeRenderTexture(render_texture_t *rt)
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

    static render_texture_t MakeRenderTexture(int width, int height,
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

        render_texture_t result = {0};
        result.width = width;
        result.height = height;
        result.num_color_attachments = num_color_attachments;

        glGenFramebuffers(1, &result.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, result.fbo);
        CheckGLError();

        result.color = new GLuint[num_color_attachments];
        for (int i = 0; i < num_color_attachments; i++)
        {
            GLuint color = TexImage2D(NULL, width, height, data_format, data_type, mag_filter, min_filter, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, internal_color_format);
            CheckGLError();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color, 0);
            result.color[i] = color;
        }

        if (enable_depth)
        {
            // DEPTH_COMPONENT16
            // DEPTH_COMPONENT24
            // DEPTH_COMPONENT32
            // DEPTH_COMPONENT32F
            glGenRenderbuffers(1, &result.depth);
            glBindRenderbuffer(GL_RENDERBUFFER, result.depth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result.depth);
        }

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        assert(status == GL_FRAMEBUFFER_COMPLETE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        CheckGLError();

        return result;
    }
}

void vdbBeginRenderTexture(int slot, int width, int height, vdbTextureFormat format, int depth_bits, int stencil_bits)
{
    using namespace render_texture;
    assert(stencil_bits == 0 && "Stencil in RenderTexture is not implemented yet.");
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");

    render_texture_t *rt = render_textures + slot;
    bool should_create = false;
    if (rt->fbo)
    {
        if (rt->width != width || rt->height != height)
        {
            FreeRenderTexture(rt);
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
        GLenum internal_format = TextureFormatToGL(format);
        bool enable_depth = (depth_bits > 0);

        *rt = MakeRenderTexture(width, height, GL_LINEAR, GL_LINEAR, enable_depth, data_format, data_type, internal_format);
        EnableRenderTexture(rt);
        glClearColor(0,0,0,0);
        if (enable_depth) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        else glClear(GL_COLOR_BUFFER_BIT);
        DisableRenderTexture(rt);
    }

    EnableRenderTexture(rt);
}

void vdbEndRenderTexture(int slot)
{
    using namespace render_texture;
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    DisableRenderTexture(render_textures + slot);

    // todo: if LINEAR_MIPMAP, regenerate mipmaps
}

void vdbBindRenderTexture(int slot)
{
    using namespace render_texture;
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    glActiveTexture(GL_TEXTURE0); // todo: let user specify this
    glBindTexture(GL_TEXTURE_2D, render_textures[slot].color[0]);
}

void vdbUnbindRenderTexture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void vdbDrawRenderTexture(int slot)
{
    using namespace render_texture;
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_textures[slot].color[0]);
    vdbTriangles();
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
