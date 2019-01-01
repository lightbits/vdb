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
