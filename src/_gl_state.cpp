void _StoreRestoreGLState(bool store)
{
    static GLint last_texture;
    static GLint last_polygon_mode[2];
    static GLint last_viewport[4];
    static GLint last_scissor_box[4];

    if (store)
    {
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
        glGetIntegerv(GL_VIEWPORT, last_viewport);
        glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    else
    {
        glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
        glPolygonMode(GL_FRONT, last_polygon_mode[0]); glPolygonMode(GL_BACK, last_polygon_mode[1]);
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();
    }
}

void StoreGLState() { _StoreRestoreGLState(true); }
void RestoreGLState() { _StoreRestoreGLState(false); }
