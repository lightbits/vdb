#pragma once
#include <stdlib.h> // malloc, free
#include <stdio.h> // printf
GLuint LoadShaderFromMemory(const char *vs, const char *fs)
{
    GLenum types[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
    const char *sources[2] = { vs, fs };
    GLuint shaders[2] = {0};
    for (int i = 0; i < 2; i++)
    {
        GLint status = 0;
        shaders[i] = glCreateShader(types[i]);
        glShaderSource(shaders[i], 1, (const GLchar **)&sources[i], 0);
        glCompileShader(shaders[i]);
        glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &status);

        // print zany driver error message if it failed
        if (!status)
        {
            GLint length;
            glGetShaderiv(shaders[i], GL_INFO_LOG_LENGTH, &length);
            char *info = (char*)malloc(length);
            glGetShaderInfoLog(shaders[i], length, NULL, info);
            printf("Failed to compile shader:\n%s\n", info);
            free(info);
            for (int j = 0; j <= i; j++)
            {
                glDeleteShader(shaders[j]);
            }
            return 0;
        }
    }

    // link and get status
    GLint status;
    GLuint program = 0;
    {
        program = glCreateProgram();
        for (int i = 0; i < 2; i++)
            glAttachShader(program, shaders[i]);
        glLinkProgram(program);
        for (int i = 0; i < 2; i++)
        {
            glDetachShader(program, shaders[i]);
            glDeleteShader(shaders[i]);
        }
        glGetProgramiv(program, GL_LINK_STATUS, &status);
    }

    if (!status)
    {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char *info = (char*)malloc(length);
        glGetProgramInfoLog(program, length, NULL, info);
        printf("Failed to link program:\n%s", info);
        free(info);
        return 0;
    }

    return program;
}
