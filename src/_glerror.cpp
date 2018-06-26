const char *GLErrorCodeString(GLenum error)
{
         if (error == GL_INVALID_ENUM)                  return "enum argument out of range";
    else if (error == GL_INVALID_VALUE)                 return "Numeric argument out of range";
    else if (error == GL_INVALID_OPERATION)             return "Operation illegal in current state";
    else if (error == GL_INVALID_FRAMEBUFFER_OPERATION) return "Framebuffer object is not complete";
    else if (error == GL_OUT_OF_MEMORY)                 return "Not enough memory left to execute command (or the driver implementation didn't bother with a proper error code)";
    return "Not an error";
}

bool CheckGLError(const char *msg="")
{
    GLenum error = glGetError();
    bool was_error = false;
    while (error != GL_NO_ERROR)
    {
        was_error = true;
        fprintf(stderr, "%s: (0x%x) %s", msg, error, GLErrorCodeString(error));
        error = glGetError();
    }
    return was_error;
}
