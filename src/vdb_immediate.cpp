#pragma once
#include "shaders/points.cpp"
#include "shaders/lines.cpp"
#include "shaders/triangles.cpp"

void vdbInverseColor(bool enable)
{
    if (enable)
    {
        glLogicOp(GL_XOR);
        glEnable(GL_COLOR_LOGIC_OP);
        vdbColor4ub(0x80, 0x80, 0x80, 0x00);
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

void vdbCullFace(bool enabled)
{
    if (enabled) glEnable(GL_CULL_FACE);
    else glDisable(GL_CULL_FACE);
}

void vdbBlendNone()
{
    glDisable(GL_BLEND);
}

void vdbBlendAdd()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
}

void vdbBlendAlpha()
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
}

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

void vdbVertex(vdbVec3 v, float w)                   { vdbVertex(v.x, v.y, v.z, w); }
void vdbVertex(vdbVec4 v)                            { vdbVertex(v.x, v.y, v.z, v.w); }
void vdbColor(vdbVec3 v, float a)                    { vdbColor(v.x, v.y, v.z, a); }
void vdbColor(vdbVec4 v)                             { vdbColor(v.x, v.y, v.z, v.w); }
void vdbVertex2fv(float *v, float z, float w)        { vdbVertex(v[0], v[1], z, w); }
void vdbVertex3fv(float *v, float w)                 { vdbVertex(v[0], v[1], v[2], w); }
void vdbVertex4fv(float *v)                          { vdbVertex(v[0], v[1], v[2], v[3]); }
void vdbColor4ubv(unsigned char *v)                  { vdbColor4ub(v[0], v[1], v[2], v[3]); }
void vdbColor3ubv(unsigned char *v, unsigned char a) { vdbColor4ub(v[0], v[1], v[2], a); }
void vdbColor4fv(float *v)                           { vdbColor(v[0], v[1], v[2], v[3]); }
void vdbColor3fv(float *v, float a)                  { vdbColor(v[0], v[1], v[2], a); }

enum imm_prim_type_t
{
    IMM_PRIM_NONE = 0,
    IMM_PRIM_POINTS,
    IMM_PRIM_LINES,
    IMM_PRIM_TRIANGLES
};

struct imm_vertex_t
{
    GLfloat position[4];
    GLfloat texel[2];
    GLubyte color[4];
};

enum { IMM_MAX_LISTS = 1024 };

struct imm_list_t
{
    GLuint vbo;
    size_t count;
    imm_prim_type_t prim_type;
    bool texel_specified;
};

struct imm_t
{
    float line_width;
    float point_size;
    int point_segments;
    bool point_size_is_3D;
    bool line_width_is_3D;

    bool initialized;
    GLuint default_texture;
    bool has_begun;

    // todo: move these members out, use e.g. default_list.
    GLuint vao;
    GLuint vbo;
    imm_prim_type_t prim_type;
    bool texel_specified;
    size_t count;
    size_t allocated_count;
    imm_vertex_t *buffer;
    imm_vertex_t vertex;

    imm_list_t *list;
    imm_list_t lists[IMM_MAX_LISTS];
};

static imm_t imm;

static void BeginImmediate(imm_prim_type_t prim_type)
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

        imm.vertex.color[0] = 0;
        imm.vertex.color[1] = 0;
        imm.vertex.color[2] = 0;
        imm.vertex.color[3] = 255;
    }

    assert(imm.initialized);
    assert(imm.buffer);
    assert(imm.vao);

    assert(!imm.has_begun && "Missing vdbEnd before vdbBegin");
    imm.has_begun = true;

    imm.prim_type = prim_type;
    imm.count = 0;
    imm.texel_specified = false;

    // We allow immColor to be called outside a Begin/End block
    // and therefore do not reset it here.
    // imm.vertex.color[0] = 0.0f;
    // imm.vertex.color[1] = 0.0f;
    // imm.vertex.color[2] = 0.0f;
    // imm.vertex.color[3] = 1.0f;

    imm.vertex.texel[0] = 0.0f;
    imm.vertex.texel[1] = 0.0f;

    imm.vertex.position[0] = 0.0f;
    imm.vertex.position[1] = 0.0f;
    imm.vertex.position[2] = 0.0f;
    imm.vertex.position[3] = 1.0f;
}

static void DrawImmediatePoints(imm_list_t list)
{
    assert(imm.vao);
    assert(imm.default_texture);

    if (!glVertexAttribDivisor)
        glVertexAttribDivisor = (GLVERTEXATTRIBDIVISORPROC)SDL_GL_GetProcAddress("glVertexAttribDivisor");
    assert(glVertexAttribDivisor && "Your system's OpenGL driver doesn't support glVertexAttribDivisor.");

    static GLuint program = 0;
    static GLint attrib_in_position = 0;
    static GLint attrib_instance_position = 0;
    static GLint attrib_instance_texel = 0;
    static GLint attrib_instance_color = 0;
    static GLint uniform_projection = 0;
    static GLint uniform_model_to_view = 0;
    static GLint uniform_sampler0 = 0;
    static GLint uniform_point_size = 0;
    static GLint uniform_size_is_3D = 0;
    if (!program)
    {
        program = LoadShaderFromMemory(shader_points_vs, shader_points_fs);

        attrib_in_position       = glGetAttribLocation(program, "in_position");
        attrib_instance_position = glGetAttribLocation(program, "instance_position");
        attrib_instance_texel    = glGetAttribLocation(program, "instance_texel");
        attrib_instance_color    = glGetAttribLocation(program, "instance_color");

        uniform_projection    = glGetUniformLocation(program, "projection");
        uniform_model_to_view = glGetUniformLocation(program, "model_to_view");
        uniform_point_size    = glGetUniformLocation(program, "point_size");
        uniform_sampler0      = glGetUniformLocation(program, "sampler0");
        uniform_size_is_3D    = glGetUniformLocation(program, "size_is_3D");
    }
    assert(program);

    glUseProgram(program);

    // set uniforms
    {
        UniformMat4(uniform_projection, 1, transform::projection);
        UniformMat4(uniform_model_to_view, 1, transform::view_model);
        glUniform1i(uniform_sampler0, 0); // We assume any user-bound texture is bound to GL_TEXTURE0
        if (!list.texel_specified)
            glBindTexture(GL_TEXTURE_2D, imm.default_texture);
        if (imm.point_size_is_3D)
        {
            // Note: point_size is treated as a radius inside the shader, but imm.point_size
            // is considered to be diameter (to be consistent with glPointSize). For efficiency
            // we divide by two before passing it in, so we don't have to do it in the shader.

            // Note: The view_model matrix may have a scaling factor. Assuming the 3x3 upper-
            // left matrix of view_model is a sequence of rotation and scaling matrices, we
            // can isolate the scaling factors like so:
            vdbMat4 TT = vdbMatTranspose(transform::view_model)*transform::view_model;
            float sx = sqrtf(TT(0,0));
            float sy = sqrtf(TT(1,1));
            // If the scaling factors are not the same, we choose the smallest one:
            float s = sx < sy ? sx : sy;

            glUniform2f(uniform_point_size, 0.5f*s*imm.point_size, 0.5f*s*imm.point_size);
        }
        else
        {
            // Convert point size units from screen pixels to NDC.
            // Note: Division by two as per above.
            glUniform2f(uniform_point_size,
                        imm.point_size/vdbGetFramebufferWidth(),
                        imm.point_size/vdbGetFramebufferHeight());
        }
        glUniform1i(uniform_size_is_3D, imm.point_size_is_3D ? 1 : 0);
    }

    // generate primitive geometry
    static int rasterization_count = 0;
    static GLenum rasterization_mode = 0;
    static GLuint point_geometry_vbo = 0;
    {
        enum { max_segments = 128 };
        static float quad[] = { -1,-1, +1,-1, +1,+1, +1,+1, -1,+1, -1,-1 };
        static vdbVec2 circle[max_segments+2];
        if (!point_geometry_vbo)
        {
            glGenBuffers(1, &point_geometry_vbo);
            assert(point_geometry_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(circle), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        if (imm.point_segments > max_segments)
            imm.point_segments = max_segments;

        int last_point_segments = 0;
        if (imm.point_segments != last_point_segments)
        {
            if (imm.point_segments == 4)
            {
                glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad), quad);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                rasterization_mode = GL_TRIANGLES;
                rasterization_count = 6;
            }
            else
            {
                circle[0] = vdbVec2(0.0f, 0.0f);
                for (int i = 0; i <= imm.point_segments; i++)
                {
                    float t = 2.0f*3.1415926f*i/(float)(imm.point_segments);
                    circle[i+1] = vdbVec2(cosf(t), sinf(t));
                }
                glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0, (imm.point_segments+2)*sizeof(vdbVec2), circle);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                rasterization_mode = GL_TRIANGLE_FAN;
                rasterization_count = imm.point_segments+2;
            }
            last_point_segments = imm.point_segments;
        }
    }
    assert(rasterization_count);
    assert(rasterization_mode);
    assert(point_geometry_vbo);

    glBindVertexArray(imm.vao); // todo: optimize. attrib format is the same each time...

    // todo: move these calls out, and optimize wrt buffer binding changes

    // instance geometry
    glBindBuffer(GL_ARRAY_BUFFER, list.vbo);
    glEnableVertexAttribArray(attrib_instance_position);
    glEnableVertexAttribArray(attrib_instance_texel);
    glEnableVertexAttribArray(attrib_instance_color);
    glVertexAttribPointer(attrib_instance_position, 4, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void*)(0));
    glVertexAttribPointer(attrib_instance_texel,    2, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void*)(4*sizeof(float)));
    glVertexAttribPointer(attrib_instance_color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(imm_vertex_t), (const void*)(6*sizeof(float)));
    glVertexAttribDivisor(attrib_instance_position, 1);
    glVertexAttribDivisor(attrib_instance_texel, 1);
    glVertexAttribDivisor(attrib_instance_color, 1);

    // primitive geometry
    glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
    glEnableVertexAttribArray(attrib_in_position);
    glVertexAttribPointer(attrib_in_position, 2, GL_FLOAT, GL_FALSE, 0, (const void*)(0));

    glDrawArraysInstanced(rasterization_mode, 0, rasterization_count, list.count);

    glDisableVertexAttribArray(attrib_in_position);
    glDisableVertexAttribArray(attrib_instance_position);
    glDisableVertexAttribArray(attrib_instance_texel);
    glDisableVertexAttribArray(attrib_instance_color);
    glVertexAttribDivisor(attrib_instance_position, 0);
    glVertexAttribDivisor(attrib_instance_texel, 0);
    glVertexAttribDivisor(attrib_instance_color, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // note: 0 is not an object in the core profile. todo: global vao? todo: ensure everyone has a vao
    glUseProgram(0); // todo: optimize
}

static void DrawImmediateLinesThin(imm_list_t list)
{
    static GLuint program = 0;
    static GLint attrib_position = 0;
    static GLint attrib_texel = 0;
    static GLint attrib_color = 0;
    static GLint uniform_sampler0 = 0;
    static GLint uniform_pvm = 0;
    if (!program)
    {
        program = LoadShaderFromMemory(shader_lines_vs, shader_lines_fs);
        attrib_position = glGetAttribLocation(program, "position");
        attrib_texel = glGetAttribLocation(program, "texel");
        attrib_color = glGetAttribLocation(program, "color");
        uniform_pvm = glGetUniformLocation(program, "pvm");
        uniform_sampler0 = glGetUniformLocation(program, "sampler0");
    }
    assert(program);

    glUseProgram(program); // todo: optimize
    UniformMat4(uniform_pvm, 1, transform::pvm);
    glUniform1i(uniform_sampler0, 0); // We assume any user-bound texture is bound to GL_TEXTURE0
    if (!list.texel_specified)
        glBindTexture(GL_TEXTURE_2D, imm.default_texture);
    glBindVertexArray(imm.vao); // todo: optimize. attrib format is the same each time...
    glBindBuffer(GL_ARRAY_BUFFER, list.vbo);
    glEnableVertexAttribArray(attrib_position);
    glEnableVertexAttribArray(attrib_texel);
    glEnableVertexAttribArray(attrib_color);
    glVertexAttribPointer(attrib_position, 4, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void*)(0));
    glVertexAttribPointer(attrib_texel,    2, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void*)(4*sizeof(float)));
    glVertexAttribPointer(attrib_color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(imm_vertex_t), (const void*)(6*sizeof(float)));
    glDrawArrays(GL_LINES, 0, list.count);
    glDisableVertexAttribArray(attrib_position);
    glDisableVertexAttribArray(attrib_texel);
    glDisableVertexAttribArray(attrib_color);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // note: 0 is not an object in the core profile. todo: global vao? todo: ensure everyone has a vao
    glUseProgram(0); // todo: optimize
}

static void DrawImmediateLinesThick(imm_list_t list)
{
    glLineWidth(imm.line_width); // todo: deprecated
    DrawImmediateLinesThin(list);
    glLineWidth(1.0f);
}

static void DrawImmediateLines(imm_list_t list)
{
    assert(imm.initialized);
    assert(list.count % 2 == 0 && "LINES type expects vertex count to be a multiple of 2");

    if (imm.line_width_is_3D || imm.line_width > 1.0f)
        DrawImmediateLinesThick(list);
    else
        DrawImmediateLinesThin(list);
}

static void DrawImmediateTriangles(imm_list_t list)
{
    assert(imm.initialized);
    assert(list.count % 3 == 0 && "TRIANGLES type expects vertex count to be a multiple of 3");

    static GLuint program = 0;
    static GLint attrib_position = 0;
    static GLint attrib_texel = 0;
    static GLint attrib_color = 0;
    static GLint uniform_sampler0 = 0;
    static GLint uniform_pvm = 0;
    if (!program)
    {
        program = LoadShaderFromMemory(shader_triangles_vs, shader_triangles_fs);
        attrib_position = glGetAttribLocation(program, "position");
        attrib_texel = glGetAttribLocation(program, "texel");
        attrib_color = glGetAttribLocation(program, "color");
        uniform_pvm = glGetUniformLocation(program, "pvm");
        uniform_sampler0 = glGetUniformLocation(program, "sampler0");
    }
    assert(program);

    glUseProgram(program); // todo: optimize
    UniformMat4(uniform_pvm, 1, transform::pvm);
    glUniform1i(uniform_sampler0, 0); // We assume any user-bound texture is bound to GL_TEXTURE0
    if (!list.texel_specified)
        glBindTexture(GL_TEXTURE_2D, imm.default_texture);
    glBindVertexArray(imm.vao); // todo: optimize. attrib format is the same each time...
    glBindBuffer(GL_ARRAY_BUFFER, list.vbo);
    glEnableVertexAttribArray(attrib_position);
    glEnableVertexAttribArray(attrib_texel);
    glEnableVertexAttribArray(attrib_color);
    glVertexAttribPointer(attrib_position, 4, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void*)(0));
    glVertexAttribPointer(attrib_texel,    2, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void*)(4*sizeof(float)));
    glVertexAttribPointer(attrib_color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(imm_vertex_t), (const void*)(6*sizeof(float)));
    glDrawArrays(GL_TRIANGLES, 0, list.count);
    glDisableVertexAttribArray(attrib_position);
    glDisableVertexAttribArray(attrib_texel);
    glDisableVertexAttribArray(attrib_color);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // note: 0 is not an object in the core profile. todo: global vao? todo: ensure everyone has a vao
    glUseProgram(0); // todo: optimize
}

static void DrawImmediate(imm_list_t list)
{
    if (list.count <= 0)
        return;
    if      (list.prim_type == IMM_PRIM_POINTS)    DrawImmediatePoints(list);
    else if (list.prim_type == IMM_PRIM_LINES)     DrawImmediateLines(list);
    else if (list.prim_type == IMM_PRIM_TRIANGLES) DrawImmediateTriangles(list);
    else assert(false);
}

void vdbEnd()
{
    assert(imm.initialized);
    assert(imm.has_begun && "Missing vdbBegin before vdbEnd");
    if (imm.count <= 0)
        return;

    if (imm.list)
    {
        if (!imm.list->vbo)
            glGenBuffers(1, &imm.list->vbo);
        assert(imm.list->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, imm.list->vbo);
        glBufferData(GL_ARRAY_BUFFER, imm.count*sizeof(imm_vertex_t), (const GLvoid*)imm.buffer, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        imm.list->texel_specified = imm.texel_specified;
        imm.list->count = imm.count;
        imm.list->prim_type = imm.prim_type;
        imm.list = NULL;
    }
    else
    {
        assert(imm.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, imm.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, imm.count*sizeof(imm_vertex_t), (const GLvoid*)imm.buffer);
        glBindBuffer(GL_ARRAY_BUFFER, 0); // todo: minimize buffer binding changes

        imm_list_t list = {0};
        list.vbo = imm.vbo;
        list.texel_specified = imm.texel_specified;
        list.count = imm.count;
        list.prim_type = imm.prim_type;
        DrawImmediate(list);
    }

    imm.has_begun = false;
}

void vdbBeginList(int slot)
{
    assert(slot >= 0 && slot < IMM_MAX_LISTS);
    assert(imm.list == NULL);
    imm.list = imm.lists + slot;
}

void vdbDrawList(int slot)
{
    assert(slot >= 0 && slot < IMM_MAX_LISTS);
    DrawImmediate(imm.lists[slot]);
}

void vdbTexel(float u, float v)
{
    assert(imm.has_begun && "vdbTexel cannot be called outside vdbBegin/vdbEnd block");
    imm.texel_specified = true;
    imm.vertex.texel[0] = u;
    imm.vertex.texel[1] = v;
}

void vdbColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    imm.vertex.color[0] = (GLubyte)(r);
    imm.vertex.color[1] = (GLubyte)(g);
    imm.vertex.color[2] = (GLubyte)(b);
    imm.vertex.color[3] = (GLubyte)(a);
}

void vdbColor(float r, float g, float b, float a)
{
    imm.vertex.color[0] = (GLubyte)(255*r);
    imm.vertex.color[1] = (GLubyte)(255*g);
    imm.vertex.color[2] = (GLubyte)(255*b);
    imm.vertex.color[3] = (GLubyte)(255*a);
}

void vdbVertex(float x, float y, float z, float w)
{
    assert(imm.has_begun && "vdbVertex cannot be called outside vdbBegin/vdbEnd block");
    assert(imm.count < imm.allocated_count);
    imm.vertex.position[0] = x;
    imm.vertex.position[1] = y;
    imm.vertex.position[2] = z;
    imm.vertex.position[3] = w;
    imm.buffer[imm.count++] = imm.vertex;

    if (imm.count == imm.allocated_count)
    {
        size_t new_allocated_count = (3*imm.allocated_count)/2;
        imm_vertex_t *new_buffer = new imm_vertex_t[new_allocated_count];
        assert(new_buffer && "Ran out of memory expanding buffer");
        for (size_t i = 0; i < imm.allocated_count; i++)
            new_buffer[i] = imm.buffer[i];
        free(imm.buffer);
        imm.buffer = new_buffer;
        imm.allocated_count = new_allocated_count;

        // We don't need to call glDeleteBuffers as per spec: "BufferData deletes any existing data store"
        glBindBuffer(GL_ARRAY_BUFFER, imm.vbo);
        glBufferData(GL_ARRAY_BUFFER, imm.allocated_count*sizeof(imm_vertex_t), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void vdbLineWidth(float width)      { imm.line_width = width; imm.line_width_is_3D = false; }
void vdbLineWidth3D(float width)    { imm.line_width = width; imm.line_width_is_3D = true; }
void vdbPointSize(float size)       { imm.point_size = size; imm.point_size_is_3D = false; }
void vdbPointSize3D(float size)     { imm.point_size = size; imm.point_size_is_3D = true; }
void vdbPointSegments(int segments) { assert(segments >= 3); imm.point_segments = segments; }
void vdbTriangles()                 { BeginImmediate(IMM_PRIM_TRIANGLES); }
void vdbBeginLines()                { BeginImmediate(IMM_PRIM_LINES); }
void vdbBeginPoints()               { BeginImmediate(IMM_PRIM_POINTS); }
void vdbLines(float line_width)     { vdbLineWidth(line_width); BeginImmediate(IMM_PRIM_LINES); }
void vdbPoints(float point_size)    { vdbPointSize(point_size); BeginImmediate(IMM_PRIM_POINTS); }

struct imm_gl_state_t
{
    GLenum last_blend_src_rgb;
    GLenum last_blend_dst_rgb;
    GLenum last_blend_src_alpha;
    GLenum last_blend_dst_alpha;
    GLenum last_blend_equation_rgb;
    GLenum last_blend_equation_alpha;
    GLboolean last_depth_writemask;
    GLboolean last_enable_blend;
    GLboolean last_enable_cull_face;
    GLboolean last_enable_depth_test;
    GLboolean last_enable_scissor_test;
    GLboolean last_enable_color_logic_op;
    float line_width;
    float point_size;
    int point_segments;
    bool line_width_is_3D;
    bool point_size_is_3D;
};

void ResetImmediateGLState()
{
    vdbLineWidth(1.0f);
    vdbPointSize(1.0f);
    vdbPointSegments(4);
    vdbBlendAlpha();
    vdbDepthWrite(false);
    vdbDepthTest(false);
    vdbCullFace(false);
    vdbInverseColor(false);
    glDisable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

imm_gl_state_t GetImmediateGLState()
{
    imm_gl_state_t s = {0};
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&s.last_blend_src_rgb);
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&s.last_blend_dst_rgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&s.last_blend_src_alpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&s.last_blend_dst_alpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&s.last_blend_equation_rgb);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&s.last_blend_equation_alpha);
    glGetBooleanv(GL_DEPTH_WRITEMASK, (GLboolean*)&s.last_depth_writemask);
    s.last_enable_blend = glIsEnabled(GL_BLEND);
    s.last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    s.last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    s.last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
    s.last_enable_color_logic_op = glIsEnabled(GL_COLOR_LOGIC_OP);
    s.line_width = imm.line_width;
    s.point_size = imm.point_size;
    s.point_segments = imm.point_segments;
    s.line_width_is_3D = imm.line_width_is_3D;
    s.point_size_is_3D = imm.point_size_is_3D;
    return s;
}

void SetImmediateGLState(imm_gl_state_t s)
{
    glBlendEquationSeparate(s.last_blend_equation_rgb, s.last_blend_equation_alpha);
    glBlendFuncSeparate(s.last_blend_src_rgb, s.last_blend_dst_rgb, s.last_blend_src_alpha, s.last_blend_dst_alpha);
    glDepthMask(s.last_depth_writemask);
    if (s.last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (s.last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (s.last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (s.last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    if (s.last_enable_color_logic_op) vdbInverseColor(true); else vdbInverseColor(false);
    imm.line_width = s.line_width;
    imm.point_size = s.point_size;
    imm.point_segments = s.point_segments;
    imm.line_width_is_3D = s.line_width_is_3D;
    imm.point_size_is_3D = s.point_size_is_3D;
}

namespace immediate
{
    static void NewFrame()
    {
        ResetImmediateGLState();
    }
}
