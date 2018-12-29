enum { max_render_textures = 1024 };
static render_texture_t render_textures[max_render_textures];

void EnableRenderTexture(render_texture_t *rt)
{
    glGetIntegerv(GL_VIEWPORT, rt->last_viewport);
    glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
    vdbViewporti(0, 0, rt->width, rt->height);
    vdb.current_render_texture = rt;
}

void DisableRenderTexture(render_texture_t *rt)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    vdbViewporti(rt->last_viewport[0], rt->last_viewport[1], rt->last_viewport[2], rt->last_viewport[3]);
    vdb.current_render_texture = NULL;
}

void vdbBeginRenderTexture(int slot, int width, int height, vdbTextureFormat format, int depth_bits, int stencil_bits)
{
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
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    DisableRenderTexture(render_textures + slot);

    // todo: if LINEAR_MIPMAP, regenerate mipmaps
}

void vdbBindRenderTexture(int slot)
{
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0); // todo: let user specify this
    glBindTexture(GL_TEXTURE_2D, render_textures[slot].color[0]);
}

void vdbUnbindRenderTexture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void vdbDrawRenderTexture(int slot)
{
    assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    StoreGLState();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, render_textures[slot].color[0]);
    glBegin(GL_TRIANGLES);
    glColor4f(1,1,1,1);
    glTexCoord2f(0,0);glVertex2f(-1,-1);
    glTexCoord2f(1,0);glVertex2f(+1,-1);
    glTexCoord2f(1,1);glVertex2f(+1,+1);
    glTexCoord2f(1,1);glVertex2f(+1,+1);
    glTexCoord2f(0,1);glVertex2f(-1,+1);
    glTexCoord2f(0,0);glVertex2f(-1,-1);
    glEnd();

    RestoreGLState();
}
