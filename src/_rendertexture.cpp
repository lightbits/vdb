#pragma once
#include <stdio.h> // printf

#define RT_MAX_COLOR_ATTACHMENTS 32

struct render_texture_t
{
    GLuint fbo; // pass to glBindFramebuffer(GL_FRAMEBUFFER, fbo)
    GLuint color[RT_MAX_COLOR_ATTACHMENTS]; // pass to glBindTexture(GL_TEXTURE_2D, color)
    GLuint depth;
    int width, height; // pass to glViewport(0,0,width,height)
    int num_color_attachments;
    GLint last_viewport[4];
};

void EnableRenderTexture(render_texture_t *rt)
{
    glGetIntegerv(GL_VIEWPORT, rt->last_viewport);
    glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
    glViewport(0, 0, rt->width, rt->height);
}

void DisableRenderTexture(render_texture_t *rt)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(rt->last_viewport[0], rt->last_viewport[1], (GLsizei)rt->last_viewport[2], (GLsizei)rt->last_viewport[3]);
}

void FreeRenderTexture(render_texture_t *rt)
{
    if (rt->fbo)   glDeleteFramebuffers(1, &rt->fbo);
    if (rt->depth) glDeleteRenderbuffers(1, &rt->depth);
    for (int i = 0; i < rt->num_color_attachments; i++)
    {
        if (rt->color[i])
            glDeleteTextures(1, &rt->color[i]);
        rt->color[i] = 0;
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
    assert(num_color_attachments <= max_color_attachments && "Number of requested color attachments exceeded GL_MAX_COLOR_ATTACHMENTS");
    assert(num_color_attachments <= RT_MAX_COLOR_ATTACHMENTS && "Number of requested color attachments exceeded hardcoded constant");

    render_texture_t result = {0};
    result.width = width;
    result.height = height;
    result.num_color_attachments = num_color_attachments;

    glGenFramebuffers(1, &result.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, result.fbo);
    CheckGLError("Creating framebuffer object");

    for (int i = 0; i < num_color_attachments; i++)
    {
        GLuint color = TexImage2D(NULL, width, height, data_format, data_type, mag_filter, min_filter, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, internal_color_format);
        CheckGLError("Creating color texture for render target");
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
    CheckGLError("Creating framebuffer");

    return result;
}
