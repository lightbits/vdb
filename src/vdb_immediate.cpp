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
void vdbPointSegments(int segments)                { assert(false && "function not supported with fixed function pipeline"); }
void vdbPointSize(float size)                      { glPointSize(size); }
void vdbPointSize3D(float size)                    { assert(false && "function not supported with fixed function pipeline"); }
void vdbBeginPoints()                              { glBegin(GL_POINTS); }
void vdbPoints(float radius)                       { glPointSize(radius); glBegin(GL_POINTS); }
void vdbTriangles()                                { glBegin(GL_TRIANGLES); }
void vdbEnd()                                      { glEnd(); }
void vdbVertex(float x, float y, float z, float w) { glVertex4f(x,y,z,w); }
void vdbColor(float r, float g, float b, float a)  { glColor4f(r,g,b,a); }
void vdbTexel(float u, float v)                    { glTexCoord2f(u,v); }

#else

#include "vdb_immediate_point_shader.cpp"

inline void UniformMatrix4fv(GLint u, int n, vdbMat4 &m)
{
    #if defined(VDB_MATRIX_ROW_MAJOR)
    glUniformMatrix4fv(u, n, GL_FALSE, m.data);
    #elif defined(VDB_MATRIX_COLUMN_MAJOR)
    glUniformMatrix4fv(u, n, GL_TRUE, m.data);
    #else
    #error "You must #define VDB_MATRIX_ROW_MAJOR or VDB_MATRIX_COLUMN_MAJOR"
    #endif
}

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

// enum { IMM_MAX_LISTS = 1024 };

// struct imm_draw_list_t
// {
//     float line_width;
//     float point_size;
//     int point_segments;
//     GLuint vbo;
//     GLuint vao;
//     int count;
//     imm_prim_type_t prim_type;
// };

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
    int point_segments;
    bool point_size_is_3D;
    bool texel_specified;
    GLuint default_texture;

    // imm_draw_list_t *list;
    // imm_draw_list_t *lists[IMM_MAX_LISTS];
};

static imm_t imm;

void BeginImmediate(imm_prim_type_t prim_type)
{
    if (!imm.initialized)
    {
        imm.initialized = true;

        if (imm.line_width == 0.0f)
            imm.line_width = 1.0f;
        if (imm.point_size == 0.0f)
            imm.point_size = 1.0f;
        if (imm.point_segments == 0)
            imm.point_segments = 4;

        imm.prim_type = IMM_PRIM_NONE;
        imm.point_size_is_3D = false;

        imm.allocated_count = 1024*100;
        imm.buffer = new imm_vertex_t[imm.allocated_count];
        assert(imm.buffer);

        imm.count = 0;

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
    assert(imm.prim_type == IMM_PRIM_NONE && "Missing immEnd before immBegin");
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

void EndImmediatePoints()
{
    assert(imm.initialized);

    if (!VertexAttribDivisor)
        VertexAttribDivisor = (GLVERTEXATTRIBDIVISORPROC)SDL_GL_GetProcAddress("glVertexAttribDivisor");
    assert(VertexAttribDivisor && "Failed to dynamically load OpenGL function.");

    static GLuint program = 0;
    static GLint attrib_in_position = 0;
    static GLint attrib_instance_position = 0;
    static GLint attrib_instance_texel = 0;
    static GLint attrib_instance_color = 0;
    static GLint uniform_projection = 0;
    static GLint uniform_model_to_view = 0;
    static GLint uniform_sampler0 = 0;
    static GLint uniform_point_size = 0;
    static GLint uniform_resolution = 0;
    static GLint uniform_size_is_3D = 0;
    if (!program)
    {
        program = LoadShaderFromMemory(immediate_point_shader_vs, immediate_point_shader_fs);

        attrib_in_position       = glGetAttribLocation(program, "in_position");
        attrib_instance_position = glGetAttribLocation(program, "instance_position");
        attrib_instance_texel    = glGetAttribLocation(program, "instance_texel");
        attrib_instance_color    = glGetAttribLocation(program, "instance_color");

        uniform_projection    = glGetUniformLocation(program, "projection");
        uniform_model_to_view = glGetUniformLocation(program, "model_to_view");
        uniform_point_size    = glGetUniformLocation(program, "point_size");
        uniform_sampler0      = glGetUniformLocation(program, "sampler0");
        uniform_resolution    = glGetUniformLocation(program, "resolution");
        uniform_size_is_3D    = glGetUniformLocation(program, "size_is_3D");
    }
    assert(program);

    glUseProgram(program);
    UniformMatrix4fv(uniform_projection, 1, transform::projection);
    UniformMatrix4fv(uniform_model_to_view, 1, transform::view_model);
    glUniform1i(uniform_sampler0, 0); // We assume any user-bound texture is bound to GL_TEXTURE0
    if (!imm.texel_specified)
        glBindTexture(GL_TEXTURE_2D, imm.default_texture);
    glUniform1f(uniform_point_size, imm.point_size);
    glUniform2f(uniform_resolution, (float)vdbGetFramebufferWidth(), (float)vdbGetFramebufferHeight());
    glUniform1i(uniform_size_is_3D, imm.point_size_is_3D ? 1 : 0);

    static GLuint point_geometry_vbo = 0;
    {
        enum { max_segments = 128 };
        static vdbVec2 circle[max_segments*3];
        if (!point_geometry_vbo)
        {
            glGenBuffers(1, &point_geometry_vbo);
            assert(point_geometry_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(circle), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        int last_point_segments = 0;
        if (imm.point_segments > max_segments)
            imm.point_segments = max_segments;
        if (imm.point_segments != last_point_segments)
        {
            for (int i = 0; i < imm.point_segments; i++)
            {
                float t1 = 2.0f*3.1415926f*(0.125f + i/(float)(imm.point_segments));
                float t2 = 2.0f*3.1415926f*(0.125f + (i+1)/(float)(imm.point_segments));
                circle[3*i + 0] = vdbVec2(0.0f, 0.0f);
                circle[3*i + 1] = vdbVec2(cosf(t1), sinf(t1));
                circle[3*i + 2] = vdbVec2(cosf(t2), sinf(t2));
            }
            last_point_segments = imm.point_segments;
            glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, imm.point_segments*3*sizeof(vdbVec2), (const GLvoid*)circle);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
        }
    }

    glBindVertexArray(imm.vao); // todo: optimize. attrib format is the same each time...
    glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
    {
        glEnableVertexAttribArray(attrib_in_position);
        glVertexAttribPointer(attrib_in_position, 2, GL_FLOAT, GL_FALSE, 0, (const void*)(0));
    }
    glBindBuffer(GL_ARRAY_BUFFER, imm.vbo);
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, imm.count*sizeof(imm_vertex_t), (const GLvoid*)imm.buffer);
        glEnableVertexAttribArray(attrib_instance_position);
        glEnableVertexAttribArray(attrib_instance_texel);
        glEnableVertexAttribArray(attrib_instance_color);
        glVertexAttribPointer(attrib_instance_position, 4, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void*)(0));
        glVertexAttribPointer(attrib_instance_texel,    2, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void*)(4*sizeof(float)));
        glVertexAttribPointer(attrib_instance_color,    4, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void*)(6*sizeof(float)));
        VertexAttribDivisor(attrib_instance_position, 1);
        VertexAttribDivisor(attrib_instance_texel, 1);
        VertexAttribDivisor(attrib_instance_color, 1);
    }
    glDrawArraysInstanced(GL_TRIANGLES, 0, imm.point_segments*3, imm.count);
    glDisableVertexAttribArray(attrib_in_position);
    glDisableVertexAttribArray(attrib_instance_position);
    glDisableVertexAttribArray(attrib_instance_texel);
    glDisableVertexAttribArray(attrib_instance_color);
    VertexAttribDivisor(attrib_instance_position, 0);
    VertexAttribDivisor(attrib_instance_texel, 0);
    VertexAttribDivisor(attrib_instance_color, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // note: 0 is not an object in the core profile. todo: global vao? todo: ensure everyone has a vao
    glUseProgram(0); // todo: optimize
}

void EndImmediateLines()
{
    #if 0
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

    glLineWidth(imm.line_width); // todo: glLineWidth(w > 1.0f) is deprecated!
    glPointSize(imm.point_size); // todo: rounded point drawing

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

    imm.prim_type = IMM_PRIM_NONE;
    assert(imm.prim_type == IMM_PRIM_NONE && "Did you forget?");
    #endif
}

void EndImmediateTriangles()
{
    #if 0
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

    glLineWidth(imm.line_width); // todo: glLineWidth(w > 1.0f) is deprecated!
    glPointSize(imm.point_size); // todo: rounded point drawing

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

    imm.prim_type = IMM_PRIM_NONE;
    assert(imm.prim_type == IMM_PRIM_NONE && "Did you forget?");
    #endif
}

void vdbEnd()
{
    assert(imm.prim_type != IMM_PRIM_NONE && "Missing immBegin before immEnd");
    assert(imm.initialized);

    if      (imm.prim_type == IMM_PRIM_LINES)     EndImmediateLines();
    else if (imm.prim_type == IMM_PRIM_POINTS)    EndImmediatePoints();
    else if (imm.prim_type == IMM_PRIM_TRIANGLES) EndImmediateTriangles();
    else assert(false);

    imm.prim_type = IMM_PRIM_NONE;
    assert(imm.prim_type == IMM_PRIM_NONE && "Did you forget?");
}

void vdbLineWidth(float width) { imm.line_width = width; }
void vdbPointSize(float size) { imm.point_size = size; imm.point_size_is_3D = false; }
void vdbPointSize3D(float size) { imm.point_size = size; imm.point_size_is_3D = true; }
void vdbPointSegments(int segments) { imm.point_segments = segments; }
void vdbTriangles() { BeginImmediate(IMM_PRIM_TRIANGLES); }
void vdbBeginLines() { BeginImmediate(IMM_PRIM_LINES); }
void vdbBeginPoints() { BeginImmediate(IMM_PRIM_POINTS); }
void vdbLines(float line_width) { vdbLineWidth(line_width); BeginImmediate(IMM_PRIM_LINES); }
void vdbPoints(float point_size) { vdbPointSize(point_size); BeginImmediate(IMM_PRIM_POINTS); }

// void vdbBeginPointsList(int slot)
// {
//     assert(slot >= 0 && slot < IMM_MAX_LISTS && "Slot out of range");
//     imm.list = imm.lists + slot;
//     imm.list->point_size = point_size;
//     imm.list->point_segments = point_segments;
//     vdbBeginPoints();
// }

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

        // todo: do we need to call glDeleteBuffers?

        glBindBuffer(GL_ARRAY_BUFFER, imm.vbo);
        glBufferData(GL_ARRAY_BUFFER, imm.allocated_count*sizeof(imm_vertex_t), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}
#endif
