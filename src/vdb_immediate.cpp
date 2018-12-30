#pragma once

void vdbInverseColor(bool enable)
{
    if (enable)
    {
        glLogicOp(GL_XOR);
        glEnable(GL_COLOR_LOGIC_OP);
        glColor4ub(0x80, 0x80, 0x80, 0x00);
    }
    else
    {
        glDisable(GL_COLOR_LOGIC_OP);
    }
}

void vdbClearColor(float r, float g, float b, float a)
{
    glClearColor(r,g,b,a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void vdbClearDepth(float d)
{
    glClearDepth(d);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void vdbBlendNone()  { glDisable(GL_BLEND); }
void vdbBlendAdd()   { glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE); }
void vdbBlendAlpha() { glEnable(GL_BLEND); glBlendEquation(GL_FUNC_ADD); glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); }

void vdbDepthTest(bool enabled)
{
    if (enabled) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
}
void vdbDepthWrite(bool enabled)
{
    if (enabled) { glDepthMask(GL_TRUE); glDepthRange(0.0f, 1.0f); }
    else { glDepthMask(GL_FALSE); }
}

void vdbVertex(vdbVec3 v, float w)                 { vdbVertex(v.x, v.y, v.z, w); }
void vdbVertex(vdbVec4 v)                          { vdbVertex(v.x, v.y, v.z, v.w); }
void vdbColor(vdbVec3 v, float a)                  { vdbColor(v.x, v.y, v.z, a); }
void vdbColor(vdbVec4 v)                           { vdbColor(v.x, v.y, v.z, v.w); }

#if VDB_USE_FIXED_FUNCTION_PIPELINE==1
// This path uses the fixed-function pipeline of legacy OpenGL.
// It is available only in compatibility profiles of OpenGL, which
// itself is not available on certain drivers (Mesa, for one).
void vdbLineWidth(float width)                     { glLineWidth(width); }
void vdbBeginLines()                               { glBegin(GL_LINES); }
void vdbLines(float width)                         { glLineWidth(width); glBegin(GL_LINES); }
void vdbLineSize(float width)                      { glLineWidth(width); }
void vdbBeginPoints()                              { glBegin(GL_POINTS); }
void vdbPoints(float radius)                       { glPointSize(radius); glBegin(GL_POINTS); }
void vdbTriangles()                                { glBegin(GL_TRIANGLES); }
void vdbEnd()                                      { glEnd(); }
void vdbVertex(float x, float y, float z, float w) { glVertex4f(x,y,z,w); }
void vdbColor(float r, float g, float b, float a)  { glColor4f(r,g,b,a); }
void vdbTexel(float u, float v)                    { glTexCoord2f(u,v); }

#else

enum imm_prim_type_t
{
    IMM_PRIM_NONE = 0,
    IMM_PRIM_POINTS,
    IMM_PRIM_LINES,
    IMM_PRIM_TRIANGLES
};

struct imm_vertex_t
{
    float position[4];
    float texel[2];
    float color[4];
};

struct imm_t
{
    bool initialized;
    GLuint vao;
    GLuint vbo;
    imm_vertex_t *buffer;
    imm_vertex_t vertex;
    size_t allocated_count;
    size_t count;
    imm_prim_type_t prim_type;
    float line_width;
    float point_size;
    bool texel_specified;
    GLuint default_texture;
};

static imm_t imm;

void BeginImmediate(imm_prim_type_t prim_type)
{
    if (!imm.initialized)
    {
        imm.initialized = true;
        imm.prim_type = IMM_PRIM_NONE;
        imm.line_width = 1.0f;
        imm.point_size = 1.0f;
        imm.count = 0;
        imm.allocated_count = 1024*100;
        imm.buffer = new imm_vertex_t[imm.allocated_count];
        assert(imm.buffer);

        glGenVertexArrays(1, &imm.vao);
        glGenBuffers(1, &imm.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, imm.vbo);
        glBufferData(GL_ARRAY_BUFFER, imm.allocated_count*sizeof(imm_vertex_t), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        static unsigned char default_texture_data[] = { 255, 255, 255, 255 };
        glGenTextures(1, &imm.default_texture);
        glBindTexture(GL_TEXTURE_2D, imm.default_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA, // internal format
                     1, // width
                     1, // height
                     0,
                     GL_RGBA, // data format
                     GL_UNSIGNED_BYTE, // data type
                     default_texture_data);
        glBindTexture(GL_TEXTURE_2D, 0);

        imm.vertex.color[0] = 0.0f;
        imm.vertex.color[1] = 0.0f;
        imm.vertex.color[2] = 0.0f;
        imm.vertex.color[3] = 1.0f;
    }

    assert(imm.buffer);
    assert(imm.initialized);
    assert(imm.vao);
    assert(imm.vbo);
    imm.prim_type = prim_type;
    imm.count = 0;

    // We allow immColor to be called outside a Begin/End block
    // and therefore do not reset it here.
    // imm.vertex.color[0] = 0.0f;
    // imm.vertex.color[1] = 0.0f;
    // imm.vertex.color[2] = 0.0f;
    // imm.vertex.color[3] = 1.0f;

    imm.vertex.texel[0] = 0.0f;
    imm.vertex.texel[1] = 0.0f;
    imm.texel_specified = false;

    imm.vertex.position[0] = 0.0f;
    imm.vertex.position[1] = 0.0f;
    imm.vertex.position[2] = 0.0f;
    imm.vertex.position[3] = 1.0f;
}

void vdbEnd()
{
    assert(imm.prim_type != IMM_PRIM_NONE && "Missing immBegin before immEnd");
    assert(imm.initialized);

    static GLuint program = 0;
    static GLint attrib_position = 0;
    static GLint attrib_texel = 0;
    static GLint attrib_color = 0;
    static GLint uniform_sampler0 = 0;
    static GLint uniform_pvm = 0;
    if (!program)
    {
        const char *vs =
            "#version 150\n"
            "in vec4 position;\n"
            "in vec2 texel;\n"
            "in vec4 color;\n"
            "uniform mat4 pvm;\n"
            "out vec4 vs_color;\n"
            "out vec2 vs_texel;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = pvm*position;\n"
            "    vs_color = color;\n"
            "    vs_texel = texel;\n"
            "}\n";

        const char *fs =
            "#version 150\n"
            "in vec4 vs_color;\n"
            "in vec2 vs_texel;\n"
            "uniform sampler2D sampler0;\n"
            "out vec4 color0;\n"
            "void main()\n"
            "{\n"
            "    color0 = vs_color*texture(sampler0, vs_texel);\n"
            "}\n";
        program = LoadShaderFromMemory(vs, fs);
        attrib_position = glGetAttribLocation(program, "position");
        attrib_texel = glGetAttribLocation(program, "texel");
        attrib_color = glGetAttribLocation(program, "color");
        uniform_pvm = glGetUniformLocation(program, "pvm");
        uniform_sampler0 = glGetUniformLocation(program, "sampler0");
    }
    assert(program);

    glLineWidth(imm.line_width); // todo: deprecated
    glPointSize(imm.point_size); // todo: deprecated

    glUseProgram(program); // todo: optimize

    #if defined(VDB_MATRIX_ROW_MAJOR)
    glUniformMatrix4fv(uniform_pvm, 1, GL_FALSE, transform::pvm.data);
    #elif defined(VDB_MATRIX_COLUMN_MAJOR)
    glUniformMatrix4fv(uniform_pvm, 1, GL_TRUE, transform::pvm.data);
    #else
    #error "You must #define VDB_MATRIX_ROW_MAJOR or VDB_MATRIX_COLUMN_MAJOR"
    #endif

    glUniform1i(uniform_sampler0, 0); // We assume any user-bound texture is bound to GL_TEXTURE0
    if (!imm.texel_specified)
        glBindTexture(GL_TEXTURE_2D, imm.default_texture);

    glBindBuffer(GL_ARRAY_BUFFER, imm.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, imm.count*sizeof(imm_vertex_t), (const GLvoid*)imm.buffer);

    glBindVertexArray(imm.vao); // todo: optimize. attrib format is the same each time...
    glEnableVertexAttribArray(attrib_position);
    glEnableVertexAttribArray(attrib_texel);
    glEnableVertexAttribArray(attrib_color);
    int stride = sizeof(imm_vertex_t);
    glVertexAttribPointer(attrib_position, 4, GL_FLOAT, GL_FALSE, stride, (const void*)(0));
    glVertexAttribPointer(attrib_texel,    2, GL_FLOAT, GL_FALSE, stride, (const void*)(4*sizeof(float)));
    glVertexAttribPointer(attrib_color,    4, GL_FLOAT, GL_FALSE, stride, (const void*)(6*sizeof(float)));
    if (imm.prim_type == IMM_PRIM_LINES)
    {
        assert(imm.count % 2 == 0 && "LINES type expects vertex count to be a multiple of 2");
        glDrawArrays(GL_LINES, 0, imm.count);
    }
    else if (imm.prim_type == IMM_PRIM_POINTS)
    {
        glDrawArrays(GL_POINTS, 0, imm.count);
    }
    else if (imm.prim_type == IMM_PRIM_TRIANGLES)
    {
        assert(imm.count % 3 == 0 && "TRIANGLES type expects vertex count to be a multiple of 3");
        glDrawArrays(GL_TRIANGLES, 0, imm.count);
    }
    else
    {
        assert(false && "unhandled primitive type");
    }
    glDisableVertexAttribArray(attrib_position);
    glDisableVertexAttribArray(attrib_texel);
    glDisableVertexAttribArray(attrib_color);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // note: 0 is not an object in the core profile. todo: global vao? todo: ensure everyone has a vao
    glLineWidth(1.0f);
    glPointSize(1.0f);
    glUseProgram(0); // todo: optimize
}

void vdbLineWidth(float width) { imm.line_width = width; }
void vdbPointSize(float size) { imm.point_size = size; }
void vdbTriangles() { BeginImmediate(IMM_PRIM_TRIANGLES); }
void vdbBeginLines() { BeginImmediate(IMM_PRIM_LINES); }
void vdbBeginPoints() { BeginImmediate(IMM_PRIM_POINTS); }
void vdbLines(float line_width) { vdbLineWidth(line_width); BeginImmediate(IMM_PRIM_LINES); }
void vdbPoints(float point_size) { vdbPointSize(point_size); BeginImmediate(IMM_PRIM_POINTS); }

void vdbTexel(float u, float v)
{
    assert(imm.prim_type != IMM_PRIM_NONE && "immTexel cannot be called outside immBegin/immEnd block");
    imm.texel_specified = true;
    imm.vertex.texel[0] = u;
    imm.vertex.texel[1] = v;
}

void vdbColor(float r, float g, float b, float a)
{
    imm.vertex.color[0] = r;
    imm.vertex.color[1] = g;
    imm.vertex.color[2] = b;
    imm.vertex.color[3] = a;
}

void vdbVertex(float x, float y, float z, float w)
{
    assert(imm.prim_type != IMM_PRIM_NONE && "immVertex cannot be called outside immBegin/immEnd block");
    imm.vertex.position[0] = x;
    imm.vertex.position[1] = y;
    imm.vertex.position[2] = z;
    imm.vertex.position[3] = w;
    imm.buffer[imm.count] = imm.vertex;
    imm.count++;

    if (imm.count == imm.allocated_count)
    {
        int new_allocated_count = (3*imm.allocated_count)/2;
        imm_vertex_t *new_buffer = new imm_vertex_t[new_allocated_count];
        assert(new_buffer && "Ran out of memory expanding buffer");
        for (size_t i = 0; i < imm.allocated_count; i++)
            new_buffer[i] = imm.buffer[i];
        free(imm.buffer);
        imm.buffer = new_buffer;
        imm.allocated_count = new_allocated_count;

        glBindBuffer(GL_ARRAY_BUFFER, imm.vbo);
        glBufferData(GL_ARRAY_BUFFER, imm.allocated_count*sizeof(imm_vertex_t), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}
#endif
