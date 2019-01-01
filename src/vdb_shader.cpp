GLuint vdb_gl_current_program = 0;
enum { vdb_max_shaders = 1000 };
static GLuint vdb_gl_shaders[vdb_max_shaders];

void vdbLoadShader(int slot, const char *fragment_shader_string)
{
    assert(slot >= 0 && slot < vdb_max_shaders && "You are trying to set a pixel shader beyond the available number of slots.");
    if (vdb_gl_shaders[slot])
        glDeleteProgram(vdb_gl_shaders[slot]);

    const char *vertex_shader_string =
    "#version 150\n"
    "in vec2 in_position;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(in_position, 0.0, 1.0);\n"
    "}\n"
    ;
    vdb_gl_shaders[slot] = LoadShaderFromMemory(vertex_shader_string, fragment_shader_string);
    assert(vdb_gl_shaders[slot] && "Failed to load shader.");
}
void vdbBeginShader(int slot)
{
    vdb_gl_current_program = vdb_gl_shaders[slot];
    glUseProgram(vdb_gl_shaders[slot]);
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
void vdbUniform1f(const char *name, float x)                            { glUniform1f(glGetUniformLocation(vdb_gl_current_program, name), x); }
void vdbUniform2f(const char *name, float x, float y)                   { glUniform2f(glGetUniformLocation(vdb_gl_current_program, name), x,y); }
void vdbUniform3f(const char *name, float x, float y, float z)          { glUniform3f(glGetUniformLocation(vdb_gl_current_program, name), x,y,z); }
void vdbUniform4f(const char *name, float x, float y, float z, float w) { glUniform4f(glGetUniformLocation(vdb_gl_current_program, name), x,y,z,w); }
void vdbUniform1i(const char *name, int x)                              { glUniform1i(glGetUniformLocation(vdb_gl_current_program, name), x); }
void vdbUniform2i(const char *name, int x, int y)                       { glUniform2i(glGetUniformLocation(vdb_gl_current_program, name), x,y); }
void vdbUniform3i(const char *name, int x, int y, int z)                { glUniform3i(glGetUniformLocation(vdb_gl_current_program, name), x,y,z); }
void vdbUniform4i(const char *name, int x, int y, int z, int w)         { glUniform4i(glGetUniformLocation(vdb_gl_current_program, name), x,y,z,w); }
void vdbUniformMatrix4fv(const char *name, float *x, bool transpose)    { glUniformMatrix4fv(glGetUniformLocation(vdb_gl_current_program, name), 1, transpose, x);}
void vdbUniformMatrix3fv(const char *name, float *x, bool transpose)    { glUniformMatrix3fv(glGetUniformLocation(vdb_gl_current_program, name), 1, transpose, x);}
