void _StoreRestoreGLState(bool store)
{
    static GLint last_texture;
    static GLint last_viewport[4];
    static GLint last_scissor_box[4];

    if (store)
    {
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_VIEWPORT, last_viewport);
        glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    }
    else
    {
        glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
        glBindTexture(GL_TEXTURE_2D, last_texture);
    }
}

void StoreGLState() { _StoreRestoreGLState(true); }
void RestoreGLState() { _StoreRestoreGLState(false); }
