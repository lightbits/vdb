struct framebuffer_t
{
    GLuint fbo; // usage: glBindFramebuffer(GL_FRAMEBUFFER, fbo)
    GLuint *color; // usage: glBindTexture(GL_TEXTURE_2D, color[i])
    GLuint depth;
    int width, height; // usage: glViewport(0, 0, width, height)
    int num_color_attachments;
    GLint last_viewport[4];
};

static framebuffer_t *current_framebuffer = NULL;

static void EnableFramebuffer(framebuffer_t *fb)
{
    glGetIntegerv(GL_VIEWPORT, fb->last_viewport);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
    vdbViewporti(0, 0, fb->width, fb->height);
    current_framebuffer = fb;
}

static void DisableFramebuffer(framebuffer_t *fb)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    vdbViewporti(fb->last_viewport[0], fb->last_viewport[1], fb->last_viewport[2], fb->last_viewport[3]);
    current_framebuffer = NULL;
}

static void FreeFramebuffer(framebuffer_t *fb)
{
    if (fb->fbo)   glDeleteFramebuffers(1, &fb->fbo);
    if (fb->depth) glDeleteRenderbuffers(1, &fb->depth);
    if (fb->color)
    {
        for (int i = 0; i < fb->num_color_attachments; i++)
            glDeleteTextures(1, &fb->color[i]);
        free(fb->color);
        fb->color = NULL;
    }
    fb->num_color_attachments = 0;
    fb->width = 0;
    fb->height = 0;
    fb->fbo = 0;
    fb->depth = 0;
}

static framebuffer_t MakeFramebuffer(
    int width, int height,
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

    framebuffer_t result = {0};
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
