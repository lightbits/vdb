#pragma once
#include <stdlib.h> // malloc, free
#include <stdio.h> // printf

static bool ShaderCompileStatus(GLuint shader)
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char *info = (char*)malloc(length);
        glGetShaderInfoLog(shader, length, NULL, info);
        fprintf(stderr, "Failed to compile shader:\n%s\n", info);
        free(info);
        return false;
    }
    return true;
}

static bool ProgramLinkStatus(GLuint program)
{
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char *info = (char*)malloc(length);
        glGetProgramInfoLog(program, length, NULL, info);
        fprintf(stderr, "Failed to link program:\n%s", info);
        free(info);
        return false;
    }
    return true;
}

static GLuint LoadShaderFromMemory(
    const char **vs_source, int num_vs_source,
    const char **fs_source, int num_fs_source)
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    {
        glShaderSource(vs, num_vs_source, vs_source, 0);
        glCompileShader(vs);
        if (!ShaderCompileStatus(vs))
        {
            glDeleteShader(vs);
            return 0;
        }
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    {
        glShaderSource(fs, num_fs_source, fs_source, 0);
        glCompileShader(fs);
        if (!ShaderCompileStatus(fs))
        {
            glDeleteShader(vs);
            glDeleteShader(fs);
            return 0;
        }
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!ProgramLinkStatus(program))
    {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

static GLuint LoadShaderFromMemory(
    const char *vs_source,
    const char *fs_source)
{
    return LoadShaderFromMemory(&vs_source, 1, &fs_source, 1);
}

GLuint vdb_gl_current_program = 0;
enum { vdb_max_shaders = 1000 };
static GLuint vdb_gl_shaders[vdb_max_shaders];

bool vdbLoadShader(int slot, const char *user_fs_source)
{
    assert(slot >= 0 && slot < vdb_max_shaders && "You are trying to set a pixel shader beyond the available number of slots.");

    const char *vs_source =
        "#version 150\n"
        "in vec2 in_position;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(in_position, 0.0, 1.0);\n"
        "}\n"
    ;

    const char *fs_source[] =
    {
        "#version 150\n"
        "uniform vec2 iResolution;\n"
        "uniform vec2 iFragCoordOffset;\n"
        "uniform mat4 iPVM;\n"
        "uniform mat4 iModelToView;\n"
        "out vec4 vdb_color0;\n"
        "#line 0\n",

        user_fs_source,

        "void main()\n"
        "{\n"
        "    vec2 fragCoord = gl_FragCoord.xy + iFragCoordOffset;\n"
        "    mainImage(vdb_color0, fragCoord);\n"
        "}\n"
    };
    int num_fs_source = sizeof(fs_source)/sizeof(fs_source[0]);

    GLuint shader = LoadShaderFromMemory(&vs_source, 1, fs_source, num_fs_source);
    if (shader == 0)
        return false;
    if (vdb_gl_shaders[slot])
        glDeleteProgram(vdb_gl_shaders[slot]);
    vdb_gl_shaders[slot] = shader;
    return true;
}

void vdbBeginShader(int slot)
{
    assert(slot >= 0 && slot < vdb_max_shaders && "Attempted to use a shader slot outside the valid range.");
    assert(glIsProgram(vdb_gl_shaders[slot]) && "Shader at specified slot is invalid.");
    vdb_gl_current_program = vdb_gl_shaders[slot];
    glUseProgram(vdb_gl_shaders[slot]);
    float pvm[4*4]; vdbGetPVM(pvm);
    float vm[4*4]; vdbGetMatrix(vm);
    vdbVec2 frag_offset = vdbGetRenderOffsetFramebuffer();
    vdbUniform2f("iResolution", (float)vdbGetFramebufferWidth(), (float)vdbGetFramebufferHeight());
    vdbUniform2f("iFragCoordOffset", frag_offset.x, frag_offset.y);
    vdbUniformMatrix4fv("iPVM", pvm);
    vdbUniformMatrix4fv("iModelToView", vm);
}

void vdbEndShader()
{
    static GLuint vao = 0;
    static GLuint vbo = 0;
    if (!vao)
    {
        static float position[] = { -1,-1, 1,-1, 1,1, 1,1, -1,1, -1,-1 };
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
    }
    assert(vao);
    assert(vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLint attrib_in_position = glGetAttribLocation(vdb_gl_current_program, "in_position");
    glEnableVertexAttribArray(attrib_in_position);
    glVertexAttribPointer(attrib_in_position, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(attrib_in_position);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    vdb_gl_current_program = 0;
}
void vdbUniform1f(const char *name, float x)                            { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniform1f(glGetUniformLocation(program, name), x);  AssertGL("vdbUniform1f failed with name '%s'\n", name); }
void vdbUniform2f(const char *name, float x, float y)                   { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniform2f(glGetUniformLocation(program, name), x,y);  AssertGL("vdbUniform2f failed with name '%s'\n", name); }
void vdbUniform3f(const char *name, float x, float y, float z)          { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniform3f(glGetUniformLocation(program, name), x,y,z);  AssertGL("vdbUniform3f failed with name '%s'\n", name); }
void vdbUniform4f(const char *name, float x, float y, float z, float w) { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniform4f(glGetUniformLocation(program, name), x,y,z,w);  AssertGL("vdbUniform4f failed with name '%s'\n", name); }
void vdbUniform1i(const char *name, int x)                              { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniform1i(glGetUniformLocation(program, name), x);  AssertGL("vdbUniform1i failed with name '%s'\n", name); }
void vdbUniform2i(const char *name, int x, int y)                       { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniform2i(glGetUniformLocation(program, name), x,y);  AssertGL("vdbUniform2i failed with name '%s'\n", name); }
void vdbUniform3i(const char *name, int x, int y, int z)                { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniform3i(glGetUniformLocation(program, name), x,y,z);  AssertGL("vdbUniform3i failed with name '%s'\n", name); }
void vdbUniform4i(const char *name, int x, int y, int z, int w)         { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniform4i(glGetUniformLocation(program, name), x,y,z,w);  AssertGL("vdbUniform4i failed with name '%s'\n", name); }
void vdbUniformMatrix4fv(const char *name, float *x)                    { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniformMatrix4fv(glGetUniformLocation(program, name), 1, false, x); AssertGL("vdbUniformMatrix4fv failed with name '%s'\n", name); }
void vdbUniformMatrix3fv(const char *name, float *x)                    { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniformMatrix3fv(glGetUniformLocation(program, name), 1, false, x); AssertGL("vdbUniformMatrix3fv failed with name '%s'\n", name); }
void vdbUniformMatrix4fv_RowMaj(const char *name, float *x)             { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniformMatrix4fv(glGetUniformLocation(program, name), 1, true, x); AssertGL("vdbUniformMatrix4fv_RowMaj failed with name '%s'\n", name); }
void vdbUniformMatrix3fv_RowMaj(const char *name, float *x)             { GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program); glUniformMatrix3fv(glGetUniformLocation(program, name), 1, true, x); AssertGL("vdbUniformMatrix3fv_RowMaj failed with name '%s'\n", name); }
