enum { max_render_textures = 1024 };
static render_texture_t render_textures[max_render_textures];

GLenum vdbTextureFormatToGL(vdbTextureFormat format)
{
    if (format == VDB_RGBA32F) return GL_RGBA32F;
    else if (format == VDB_RGBA8U) return GL_RGBA8;

    SDL_assert(false && "Unrecognized vdbTextureFormat");
    return GL_RGBA;
}

void vdbBeginRenderTexture(int slot, int width, int height, vdbTextureFormat format, int depth_bits, int stencil_bits)
{
    SDL_assert(stencil_bits == 0 && "Stencil in RenderTexture is not implemented yet.");
    SDL_assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");

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
        GLenum internal_format = vdbTextureFormatToGL(format);
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
    SDL_assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    DisableRenderTexture(render_textures + slot);

    // todo: if LINEAR_MIPMAP, regenerate mipmaps
}

void vdbDrawRenderTexture(int slot, vdbTextureFilter filter)
{
    SDL_assert(slot >= 0 && slot < max_render_textures && "You are trying to use a render texture beyond the available slots.");
    SDL_assert(filter != VDB_LINEAR_MIPMAP && "Mipmapping not supported with render textures");
    StoreGLState();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, render_textures[slot].color[0]);
    GLenum mag_filter = filter == VDB_NEAREST ? GL_NEAREST : GL_LINEAR;
    GLenum min_filter = filter == VDB_NEAREST ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
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
