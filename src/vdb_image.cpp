void vdbLoadImageUint8(int slot, const void *data, int width, int height, int channels)
{
    SDL_assert(channels >= 1 && channels <= 4 && "'channels' must be 1,2,3 or 4");
    if (channels == 1)      SetTexture(slot, data, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
    else if (channels == 2) SetTexture(slot, data, width, height, GL_RG, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
    else if (channels == 3) SetTexture(slot, data, width, height, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
    else if (channels == 4) SetTexture(slot, data, width, height, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
}
void vdbLoadImageFloat32(int slot, const void *data, int width, int height, int channels)
{
    SDL_assert(channels >= 1 && channels <= 4 && "'channels' must be 1,2,3 or 4");
    if (channels == 1)      SetTexture(slot, data, width, height, GL_LUMINANCE, GL_FLOAT, GL_NEAREST, GL_NEAREST);
    else if (channels == 2) SetTexture(slot, data, width, height, GL_RG, GL_FLOAT, GL_NEAREST, GL_NEAREST);
    else if (channels == 3) SetTexture(slot, data, width, height, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST);
    else if (channels == 4) SetTexture(slot, data, width, height, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST);
}
void vdbLoadImageFromFile(int slot, const char *filename, int *width, int *height, int *channels)
{
    int w,h,n;
    unsigned char *data = stbi_load(filename, &w, &h, &n, 4);
    SDL_assert(data && "Failed to load image from file.");
    SetTexture(slot, data, w, h, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    free(data);

    if (width) *width = w;
    if (height) *height = h;
    if (channels) *channels = n;
}
void vdbDrawImage(int slot)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, GetTextureSlotHandle(slot));
    glBegin(GL_TRIANGLES);
    #if VDB_FLIP_IMAGE_TEXEL_Y==1
    glColor4f(1,1,1,1); glTexCoord2f(0,1);glVertex2f(-1,-1);
    glColor4f(1,1,1,1); glTexCoord2f(1,1);glVertex2f(+1,-1);
    glColor4f(1,1,1,1); glTexCoord2f(1,0);glVertex2f(+1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(1,0);glVertex2f(+1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(0,0);glVertex2f(-1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(0,1);glVertex2f(-1,-1);
    #else
    glColor4f(1,1,1,1); glTexCoord2f(0,0);glVertex2f(-1,-1);
    glColor4f(1,1,1,1); glTexCoord2f(1,0);glVertex2f(+1,-1);
    glColor4f(1,1,1,1); glTexCoord2f(1,1);glVertex2f(+1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(1,1);glVertex2f(+1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(0,1);glVertex2f(-1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(0,0);glVertex2f(-1,-1);
    #endif
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
void vdbBindImage(int slot)
{
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureSlotHandle(slot));
}
void vdbUnbindImage(int slot)
{
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}
