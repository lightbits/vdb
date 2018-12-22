#pragma once

struct render_texture_t
{
    GLuint fbo; // usage: glBindFramebuffer(GL_FRAMEBUFFER, fbo)
    GLuint *color; // usage: glBindTexture(GL_TEXTURE_2D, color[i])
    GLuint depth;
    int width, height; // usage: glViewport(0, 0, width, height)
    int num_color_attachments;
    GLint last_viewport[4];
};

void FreeRenderTexture(render_texture_t *rt)
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

render_texture_t MakeRenderTexture(int width, int height,
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
