const char *GLErrorCodeString(GLenum error)
{
         if (error == GL_INVALID_ENUM)                  return "GL_INVALID_ENUM";
    else if (error == GL_INVALID_VALUE)                 return "GL_INVALID_VALUE";
    else if (error == GL_INVALID_OPERATION)             return "GL_INVALID_OPERATION";
    else if (error == GL_INVALID_FRAMEBUFFER_OPERATION) return "GL_INVALID_FRAMEBUFFER_OPERATION";
    else if (error == GL_OUT_OF_MEMORY)                 return "GL_OUT_OF_MEMORY";
    return "Not an error";
}

#define CheckGLError() { \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        fprintf(stderr, "OpenGL error in file %s line %d: (0x%x) %s\n", __FILE__, __LINE__, error, GLErrorCodeString(error)); \
        exit(EXIT_FAILURE); \
    } \
}
