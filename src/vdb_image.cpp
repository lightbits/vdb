void vdbLoadImageUint8(int slot, const void *data, int width, int height, int channels)
{
    assert(channels >= 1 && channels <= 4 && "'channels' must be 1,2,3 or 4");
    if (channels == 1)      SetTexture(slot, data, width, height, GL_RED, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
    else if (channels == 2) SetTexture(slot, data, width, height, GL_RG, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
    else if (channels == 3) SetTexture(slot, data, width, height, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
    else if (channels == 4) SetTexture(slot, data, width, height, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
}
void vdbLoadImageFloat32(int slot, const void *data, int width, int height, int channels)
{
    assert(channels >= 1 && channels <= 4 && "'channels' must be 1,2,3 or 4");
    if (channels == 1)      SetTexture(slot, data, width, height, GL_RED, GL_FLOAT, GL_NEAREST, GL_NEAREST);
    else if (channels == 2) SetTexture(slot, data, width, height, GL_RG, GL_FLOAT, GL_NEAREST, GL_NEAREST);
    else if (channels == 3) SetTexture(slot, data, width, height, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST);
    else if (channels == 4) SetTexture(slot, data, width, height, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST);
}
void vdbLoadImageFromFile(int slot, const char *filename, int *width, int *height, int *channels)
{
    int w,h,n;
    unsigned char *data = stbi_load(filename, &w, &h, &n, 4);
    assert(data && "Failed to load image from file.");
    SetTexture(slot, data, w, h, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    free(data);

    if (width) *width = w;
    if (height) *height = h;
    if (channels) *channels = n;
}
void vdbDrawImage(int slot)
{
    glBindTexture(GL_TEXTURE_2D, GetTextureSlotHandle(slot));
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
void vdbBindImage(int slot)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetTextureSlotHandle(slot));
}
void vdbUnbindImage()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
