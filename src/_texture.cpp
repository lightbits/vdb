#pragma once

GLenum TextureFormatToGL(vdbTextureFormat format)
{
    if (format == VDB_RGBA32F) return GL_RGBA32F;
    else if (format == VDB_RGBA8U) return GL_RGBA8;
    SDL_assert(false && "Unrecognized vdbTextureFormat");
    return GL_RGBA;
}

GLenum TextureFilterToGL(vdbTextureFilter filter)
{
    if (filter == VDB_NEAREST) return GL_NEAREST;
    else if (filter == VDB_LINEAR) return GL_LINEAR;
    else if (filter == VDB_LINEAR_MIPMAP) return GL_LINEAR_MIPMAP_LINEAR;
    SDL_assert(false && "Unrecognized vdbTextureFilter");
    return GL_NEAREST;
}

GLenum TextureWrapToGL(vdbTextureWrap wrap)
{
    if (wrap == VDB_CLAMP) return GL_CLAMP_TO_EDGE;
    else if (wrap == VDB_REPEAT) return GL_REPEAT;
    SDL_assert(false && "Unrecognized vdbTextureWrap");
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

// TEXTURES
//   Can be assigned to a 'slot' which can then be bound to render it on meshes.
//   OpenGL supports many texture formats aside from unsigned 8 bit values, for example,
//   32bit float or 32bit int, as well as anywhere from one to four components (RGBA).
// EXAMPLE
//   unsigned char data[128*128*3];
//   SetTexture(0, data, 128, 128, GL_RGB, GL_UNSIGNED_BYTE);
//   DrawTexture(0);
// EXAMPLE
//   Drawing a mipmap'd texture
//   SetTexture(0, data, 128, 128, GL_RGB, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
// EXAMPLE
//                           data_format   data_type
//   Grayscale 32 bit float: GL_LUMINANCE  GL_FLOAT
//   RGB       32 bit float: GL_RGB        GL_FLOAT
//   RGB        8 bit  char: GL_RGB        GL_UNSIGNED_BYTE
void SetTexture(
    int slot,
    const void *data,
    int width,
    int height,
    GLenum data_format = GL_RGB,
    GLenum data_type = GL_UNSIGNED_BYTE,
    GLenum mag_filter = GL_LINEAR,
    GLenum min_filter = GL_LINEAR,
    GLenum wrap_s = GL_CLAMP_TO_EDGE,
    GLenum wrap_t = GL_CLAMP_TO_EDGE,
    GLenum internal_format = GL_RGBA);
void DrawTexture(int slot); // Draws the texture to the entire viewport
void BindTexture(int slot); // Enable a texture for custom drawing

//
// Implementation
//

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

GLuint GetTextureSlotHandle(int slot)
{
    return 4040 + slot;
}

void SetTexture(
    int slot,
    const void *data,
    int width,
    int height,
    GLenum data_format,
    GLenum data_type,
    GLenum mag_filter,
    GLenum min_filter,
    GLenum wrap_s,
    GLenum wrap_t,
    GLenum internal_format)
{
    GLuint tex = GetTextureSlotHandle(slot); // Hopefully no one else uses this texture range!
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    if (min_filter == GL_LINEAR_MIPMAP_LINEAR)
    {
        glGenerateMipmap(GL_TEXTURE_2D); // todo
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    }
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

void BindTexture(int slot)
{
    glBindTexture(GL_TEXTURE_2D, GetTextureSlotHandle(slot));
}

void DrawTexture(int slot)
{
    glEnable(GL_TEXTURE_2D);
    BindTexture(slot);
    glBegin(GL_TRIANGLES);
    glColor4f(1,1,1,1); glTexCoord2f(0,0); glVertex2f(-1,-1);
    glColor4f(1,1,1,1); glTexCoord2f(1,0); glVertex2f(+1,-1);
    glColor4f(1,1,1,1); glTexCoord2f(1,1); glVertex2f(+1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(1,1); glVertex2f(+1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(0,1); glVertex2f(-1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(0,0); glVertex2f(-1,-1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
