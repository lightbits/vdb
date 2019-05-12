#pragma once
#include "shaders/points.h"
#include "shaders/lines.h"
#include "shaders/thick_lines.h"
#include "shaders/triangles.h"

typedef void (APIENTRYP GLVERTEXATTRIBDIVISORPROC)(GLuint, GLuint);
GLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;

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
    size_t count;
    size_t vbo_capacity;
    GLuint vbo;
    imm_prim_type_t prim_type;
    bool texel_specified;
};

struct imm_state_t
{
    GLenum blend_src_rgb;
    GLenum blend_dst_rgb;
    GLenum blend_src_alpha;
    GLenum blend_dst_alpha;
    GLenum blend_equation_rgb;
    GLenum blend_equation_alpha;
    GLboolean depth_writemask;
    GLboolean enable_blend;
    GLboolean enable_cull_face;
    GLboolean enable_depth_test;
    GLboolean enable_scissor_test;
    GLboolean enable_color_logic_op;
    float line_width;
    float point_size;
    int point_segments;
    bool line_width_is_3D;
    bool point_size_is_3D;
};

struct imm_t
{
    imm_state_t state;

    bool initialized;
    GLuint default_texture;
    bool inside_begin_end;

    size_t buffer_capacity;
    imm_vertex_t *buffer;
    size_t count;
    imm_vertex_t vertex;
    bool texel_specified;
    imm_prim_type_t prim_type;

    GLuint vao;
    bool is_reusable_list;
    imm_list_t *current_list;
    imm_list_t default_list;
    imm_list_t user_lists[IMM_MAX_LISTS];

    vdbVec2 ndc_offset;
};

static imm_t imm;

namespace immediate
{
    static bool clear_color_was_set;
    static vdbVec4 clear_color;

    static void DefaultState()
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

    static imm_state_t GetState()
    {
        imm_state_t s = {0};
        glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&s.blend_src_rgb);
        glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&s.blend_dst_rgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&s.blend_src_alpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&s.blend_dst_alpha);
        glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&s.blend_equation_rgb);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&s.blend_equation_alpha);
        glGetBooleanv(GL_DEPTH_WRITEMASK, (GLboolean*)&s.depth_writemask);
        s.enable_blend = glIsEnabled(GL_BLEND);
        s.enable_cull_face = glIsEnabled(GL_CULL_FACE);
        s.enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
        s.enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
        s.enable_color_logic_op = glIsEnabled(GL_COLOR_LOGIC_OP);
        s.line_width = imm.state.line_width;
        s.point_size = imm.state.point_size;
        s.point_segments = imm.state.point_segments;
        s.line_width_is_3D = imm.state.line_width_is_3D;
        s.point_size_is_3D = imm.state.point_size_is_3D;
        return s;
    }

    static void SetState(imm_state_t s)
    {
        glBlendEquationSeparate(s.blend_equation_rgb, s.blend_equation_alpha);
        glBlendFuncSeparate(s.blend_src_rgb, s.blend_dst_rgb, s.blend_src_alpha, s.blend_dst_alpha);
        glDepthMask(s.depth_writemask);
        if (s.enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (s.enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (s.enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (s.enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        if (s.enable_color_logic_op) vdbInverseColor(true); else vdbInverseColor(false);
        imm.state.line_width = s.line_width;
        imm.state.point_size = s.point_size;
        imm.state.point_segments = s.point_segments;
        imm.state.line_width_is_3D = s.line_width_is_3D;
        imm.state.point_size_is_3D = s.point_size_is_3D;
    }

    static void SetRenderOffsetNDC(vdbVec2 ndc_offset)
    {
        imm.ndc_offset = ndc_offset;
    }

    static void NewFrame()
    {
        immediate::DefaultState();
        immediate::clear_color_was_set = false;
    }
}

static void BeginImmediate(imm_prim_type_t prim_type)
{
    if (!imm.initialized)
    {
        imm.initialized = true;

        if (imm.state.line_width == 0.0f)
            imm.state.line_width = 1.0f;
        if (imm.state.point_size == 0.0f)
            imm.state.point_size = 1.0f;
        if (imm.state.point_segments == 0)
            imm.state.point_segments = 4;

        imm.buffer_capacity = 1024*100;
        imm.buffer = new imm_vertex_t[imm.buffer_capacity];
        assert(imm.buffer);

        glGenVertexArrays(1, &imm.vao);

        static unsigned char default_texture_data[] = { 255, 255, 255, 255 };
        glGenTextures(1, &imm.default_texture);
        glBindTexture(GL_TEXTURE_2D, imm.default_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1,  1,  0, GL_RGBA, GL_UNSIGNED_BYTE, default_texture_data);
        glBindTexture(GL_TEXTURE_2D, 0);

        imm.vertex.color[0] = 0;
        imm.vertex.color[1] = 0;
        imm.vertex.color[2] = 0;
        imm.vertex.color[3] = 255;
    }

    assert(imm.initialized);
    assert(imm.buffer);
    assert(imm.vao);

    assert(!imm.inside_begin_end && "Missing vdbEnd before vdbBegin");
    imm.inside_begin_end = true;
    imm.prim_type = prim_type;
    imm.count = 0;
    imm.texel_specified = false;

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

    static GLuint program = LoadShaderFromMemory(shader_points_vs, shader_points_fs);
    assert(program);
    static GLint attrib_in_position       = glGetAttribLocation(program, "in_position");
    static GLint attrib_instance_position = glGetAttribLocation(program, "instance_position");
    static GLint attrib_instance_texel    = glGetAttribLocation(program, "instance_texel");
    static GLint attrib_instance_color    = glGetAttribLocation(program, "instance_color");
    static GLint uniform_projection       = glGetUniformLocation(program, "projection");
    static GLint uniform_model_to_view    = glGetUniformLocation(program, "model_to_view");
    static GLint uniform_point_size       = glGetUniformLocation(program, "point_size");
    static GLint uniform_sampler0         = glGetUniformLocation(program, "sampler0");
    static GLint uniform_ndc_offset       = glGetUniformLocation(program, "ndc_offset");
    static GLint uniform_size_is_3D       = glGetUniformLocation(program, "size_is_3D");

    glUseProgram(program);

    // set uniforms
    {
        UniformMat4(uniform_projection, 1, transform::projection);
        UniformMat4(uniform_model_to_view, 1, transform::view_model);
        glUniform1i(uniform_sampler0, 0); // We assume any user-bound texture is bound to GL_TEXTURE0
        if (!list.texel_specified)
            glBindTexture(GL_TEXTURE_2D, imm.default_texture);
        if (imm.state.point_size_is_3D)
        {
            // Note: point_size is treated as a radius inside the shader, but imm.state.point_size
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

            glUniform2f(uniform_point_size, 0.5f*s*imm.state.point_size, 0.5f*s*imm.state.point_size);
        }
        else
        {
            // Convert point size units from screen pixels to NDC.
            // Note: Division by two as per above.
            glUniform2f(uniform_point_size,
                        imm.state.point_size/vdbGetWindowWidth(),
                        imm.state.point_size/vdbGetWindowHeight());
        }
        glUniform1i(uniform_size_is_3D, imm.state.point_size_is_3D ? 1 : 0);
        glUniform2f(uniform_ndc_offset, imm.ndc_offset.x, imm.ndc_offset.y);
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

        if (imm.state.point_segments > max_segments)
            imm.state.point_segments = max_segments;

        int last_point_segments = 0;
        if (imm.state.point_segments != last_point_segments)
        {
            if (imm.state.point_segments == 4)
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
                for (int i = 0; i <= imm.state.point_segments; i++)
                {
                    float t = 2.0f*3.1415926f*i/(float)(imm.state.point_segments);
                    circle[i+1] = vdbVec2(cosf(t), sinf(t));
                }
                glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0, (imm.state.point_segments+2)*sizeof(vdbVec2), circle);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                rasterization_mode = GL_TRIANGLE_FAN;
                rasterization_count = imm.state.point_segments+2;
            }
            last_point_segments = imm.state.point_segments;
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
    static GLuint program = LoadShaderFromMemory(shader_lines_vs, shader_lines_fs);
    assert(program);
    static GLint attrib_position  = glGetAttribLocation(program, "position");
    static GLint attrib_texel     = glGetAttribLocation(program, "texel");
    static GLint attrib_color     = glGetAttribLocation(program, "color");
    static GLint uniform_pvm      = glGetUniformLocation(program, "pvm");
    static GLint uniform_sampler0 = glGetUniformLocation(program, "sampler0");

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
    assert(imm.vao);
    assert(imm.default_texture);

    if (!glVertexAttribDivisor)
        glVertexAttribDivisor = (GLVERTEXATTRIBDIVISORPROC)SDL_GL_GetProcAddress("glVertexAttribDivisor");
    assert(glVertexAttribDivisor && "Your system's OpenGL driver doesn't support glVertexAttribDivisor.");

    static GLuint program = LoadShaderFromMemory(shader_thick_lines_vs, shader_thick_lines_fs);
    assert(program);

    static GLint attrib_in_position        = glGetAttribLocation(program, "in_position");
    static GLint attrib_instance_position0 = glGetAttribLocation(program, "instance_position0");
    static GLint attrib_instance_texel0    = glGetAttribLocation(program, "instance_texel0");
    static GLint attrib_instance_color0    = glGetAttribLocation(program, "instance_color0");
    static GLint attrib_instance_position1 = glGetAttribLocation(program, "instance_position1");
    static GLint attrib_instance_texel1    = glGetAttribLocation(program, "instance_texel1");
    static GLint attrib_instance_color1    = glGetAttribLocation(program, "instance_color1");

    static GLint uniform_projection        = glGetUniformLocation(program, "projection");
    static GLint uniform_model_to_view     = glGetUniformLocation(program, "model_to_view");
    static GLint uniform_line_width        = glGetUniformLocation(program, "line_width");
    static GLint uniform_aspect            = glGetUniformLocation(program, "aspect");
    static GLint uniform_sampler0          = glGetUniformLocation(program, "sampler0");
    static GLint uniform_ndc_offset        = glGetUniformLocation(program, "ndc_offset");
    static GLint uniform_width_is_3D       = glGetUniformLocation(program, "width_is_3D");

    glUseProgram(program);

    // set uniforms
    {
        UniformMat4(uniform_projection, 1, transform::projection);
        UniformMat4(uniform_model_to_view, 1, transform::view_model);
        glUniform1i(uniform_sampler0, 0); // We assume any user-bound texture is bound to GL_TEXTURE0
        if (!list.texel_specified)
            glBindTexture(GL_TEXTURE_2D, imm.default_texture);
        if (imm.state.line_width_is_3D)
        {
            assert(false && "Not implemented yet");
            // Note: point_size is treated as a radius inside the shader, but imm.state.point_size
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

            glUniform2f(uniform_line_width, 0.5f*s*imm.state.line_width, 0.5f*s*imm.state.line_width);
        }
        else
        {
            // Convert point size units from screen pixels to NDC.
            // Note: Division by two as per above.
            glUniform2f(uniform_line_width,
                        imm.state.line_width/vdbGetWindowWidth(),
                        imm.state.line_width/vdbGetWindowHeight());
        }
        glUniform1f(uniform_aspect, (float)vdbGetFramebufferWidth()/vdbGetFramebufferHeight());
        glUniform1i(uniform_width_is_3D, imm.state.line_width_is_3D ? 1 : 0);
        glUniform2f(uniform_ndc_offset, imm.ndc_offset.x, imm.ndc_offset.y);
    }

    // generate primitive geometry
    static int rasterization_count = 0;
    static GLenum rasterization_mode = 0;
    static GLuint point_geometry_vbo = 0;
    if (!point_geometry_vbo)
    {
        static float quad[] = { -1,-1, +1,-1, +1,+1, +1,+1, -1,+1, -1,-1 };
        glGenBuffers(1, &point_geometry_vbo);
        assert(point_geometry_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        rasterization_mode = GL_TRIANGLES;
        rasterization_count = 6;
    }
    assert(rasterization_count);
    assert(rasterization_mode);
    assert(point_geometry_vbo);

    glBindVertexArray(imm.vao); // todo: optimize. attrib format is the same each time...

    // todo: move these calls out, and optimize wrt buffer binding changes

    // instance geometry
    glBindBuffer(GL_ARRAY_BUFFER, list.vbo);
    glEnableVertexAttribArray(attrib_instance_position0);
    glEnableVertexAttribArray(attrib_instance_texel0);
    glEnableVertexAttribArray(attrib_instance_color0);
    glEnableVertexAttribArray(attrib_instance_position1);
    glEnableVertexAttribArray(attrib_instance_texel1);
    glEnableVertexAttribArray(attrib_instance_color1);
    glVertexAttribPointer(attrib_instance_position0, 4, GL_FLOAT, GL_FALSE,        2*sizeof(imm_vertex_t), (const void*)(0));
    glVertexAttribPointer(attrib_instance_texel0,    2, GL_FLOAT, GL_FALSE,        2*sizeof(imm_vertex_t), (const void*)(4*sizeof(float)));
    glVertexAttribPointer(attrib_instance_color0,    4, GL_UNSIGNED_BYTE, GL_TRUE, 2*sizeof(imm_vertex_t), (const void*)(6*sizeof(float)));
    glVertexAttribPointer(attrib_instance_position1, 4, GL_FLOAT, GL_FALSE,        2*sizeof(imm_vertex_t), (const void*)(0 + sizeof(imm_vertex_t)));
    glVertexAttribPointer(attrib_instance_texel1,    2, GL_FLOAT, GL_FALSE,        2*sizeof(imm_vertex_t), (const void*)(4*sizeof(float) + sizeof(imm_vertex_t)));
    glVertexAttribPointer(attrib_instance_color1,    4, GL_UNSIGNED_BYTE, GL_TRUE, 2*sizeof(imm_vertex_t), (const void*)(6*sizeof(float) + sizeof(imm_vertex_t)));
    glVertexAttribDivisor(attrib_instance_position0, 1);
    glVertexAttribDivisor(attrib_instance_texel0, 1);
    glVertexAttribDivisor(attrib_instance_color0, 1);
    glVertexAttribDivisor(attrib_instance_position1, 1);
    glVertexAttribDivisor(attrib_instance_texel1, 1);
    glVertexAttribDivisor(attrib_instance_color1, 1);

    // primitive geometry
    glBindBuffer(GL_ARRAY_BUFFER, point_geometry_vbo);
    glEnableVertexAttribArray(attrib_in_position);
    glVertexAttribPointer(attrib_in_position, 2, GL_FLOAT, GL_FALSE, 0, (const void*)(0));

    glDrawArraysInstanced(rasterization_mode, 0, rasterization_count, list.count/2);

    glDisableVertexAttribArray(attrib_in_position);

    glDisableVertexAttribArray(attrib_instance_position0);
    glDisableVertexAttribArray(attrib_instance_texel0);
    glDisableVertexAttribArray(attrib_instance_color0);
    glDisableVertexAttribArray(attrib_instance_position1);
    glDisableVertexAttribArray(attrib_instance_texel1);
    glDisableVertexAttribArray(attrib_instance_color1);
    glVertexAttribDivisor(attrib_instance_position0, 0);
    glVertexAttribDivisor(attrib_instance_texel0, 0);
    glVertexAttribDivisor(attrib_instance_color0, 0);
    glVertexAttribDivisor(attrib_instance_position1, 0);
    glVertexAttribDivisor(attrib_instance_texel1, 0);
    glVertexAttribDivisor(attrib_instance_color1, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // note: 0 is not an object in the core profile. todo: global vao? todo: ensure everyone has a vao
    glUseProgram(0); // todo: optimize
}

static void DrawImmediateLines(imm_list_t list)
{
    assert(imm.initialized);
    assert(list.count % 2 == 0 && "LINES type expects vertex count to be a multiple of 2");

    bool use_thick_shader =
        imm.state.line_width_is_3D ||
        imm.state.line_width != 1.0f ||
        vdbGetRenderScale().x != 1.0f ||
        vdbGetRenderScale().y != 1.0f;

    if (use_thick_shader)
        DrawImmediateLinesThick(list);
    else
        DrawImmediateLinesThin(list);
}

static void DrawImmediateTriangles(imm_list_t list)
{
    assert(imm.initialized);
    assert(list.count % 3 == 0 && "TRIANGLES type expects vertex count to be a multiple of 3");

    static GLuint program = LoadShaderFromMemory(shader_triangles_vs, shader_triangles_fs);
    assert(program);
    static GLint attrib_position  = glGetAttribLocation(program, "position");
    static GLint attrib_texel     = glGetAttribLocation(program, "texel");
    static GLint attrib_color     = glGetAttribLocation(program, "color");
    static GLint uniform_pvm      = glGetUniformLocation(program, "pvm");
    static GLint uniform_sampler0 = glGetUniformLocation(program, "sampler0");
    static GLint ndc_offset       = glGetUniformLocation(program, "ndc_offset");

    glUseProgram(program); // todo: optimize
    UniformMat4(uniform_pvm, 1, transform::pvm);
    glUniform1i(uniform_sampler0, 0); // We assume any user-bound texture is bound to GL_TEXTURE0
    glUniform2f(ndc_offset, imm.ndc_offset.x, imm.ndc_offset.y);
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
    assert(imm.inside_begin_end && "Missing vdbBegin before vdbEnd");

    if (imm.count <= 0)
    {
        imm.inside_begin_end = false;
        imm.current_list = NULL;
        return;
    }

    imm_list_t *list = &imm.default_list;
    GLenum vbo_mode = GL_STATIC_DRAW;
    if (imm.current_list)
    {
        list = imm.current_list;
        vbo_mode = GL_DYNAMIC_DRAW;
    }

    if (!list->vbo)
        glGenBuffers(1, &list->vbo);
    assert(list->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, list->vbo);
    if (list->vbo_capacity >= imm.count)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, imm.count*sizeof(imm_vertex_t), (const GLvoid*)imm.buffer);
    }
    else
    {
        // We don't need to call glDeleteBuffers as per spec: "BufferData deletes any existing data store"
        glBufferData(GL_ARRAY_BUFFER, imm.count*sizeof(imm_vertex_t), (const GLvoid*)imm.buffer, vbo_mode);
        list->vbo_capacity = imm.count;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    list->texel_specified = imm.texel_specified;
    list->count = imm.count;
    list->prim_type = imm.prim_type;

    if (!imm.current_list)
        DrawImmediate(*list);

    imm.inside_begin_end = false;
    imm.current_list = NULL;
}

void vdbBeginList(int slot)
{
    assert(slot >= 0 && slot < IMM_MAX_LISTS);
    assert(imm.current_list == NULL);
    imm.current_list = imm.user_lists + slot;
}

void vdbDrawList(int slot)
{
    assert(slot >= 0 && slot < IMM_MAX_LISTS);
    DrawImmediate(imm.user_lists[slot]);
}

void vdbTexel(float u, float v)
{
    assert(imm.inside_begin_end && "vdbTexel cannot be called outside vdbBegin/vdbEnd block");
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
    assert(imm.inside_begin_end && "vdbVertex cannot be called outside vdbBegin/vdbEnd block");
    assert(imm.count < imm.buffer_capacity);
    imm.vertex.position[0] = x;
    imm.vertex.position[1] = y;
    imm.vertex.position[2] = z;
    imm.vertex.position[3] = w;
    imm.buffer[imm.count++] = imm.vertex;

    if (imm.count == imm.buffer_capacity)
    {
        size_t new_buffer_capacity = (3*imm.buffer_capacity)/2;
        imm_vertex_t *new_buffer = new imm_vertex_t[new_buffer_capacity];
        assert(new_buffer && "Ran out of memory expanding buffer");
        for (size_t i = 0; i < imm.buffer_capacity; i++)
            new_buffer[i] = imm.buffer[i];
        free(imm.buffer);
        imm.buffer = new_buffer;
        imm.buffer_capacity = new_buffer_capacity;
    }
}

void vdbLineWidth(float width)                       { imm.state.line_width = width; imm.state.line_width_is_3D = false; }
void vdbLineWidth3D(float width)                     { imm.state.line_width = width; imm.state.line_width_is_3D = true; }
void vdbPointSize(float size)                        { imm.state.point_size = size; imm.state.point_size_is_3D = false; }
void vdbPointSize3D(float size)                      { imm.state.point_size = size; imm.state.point_size_is_3D = true; }
void vdbPointSegments(int segments)                  { assert(segments >= 3); imm.state.point_segments = segments; }
void vdbBeginTriangles()                             { BeginImmediate(IMM_PRIM_TRIANGLES); }
void vdbBeginLines()                                 { BeginImmediate(IMM_PRIM_LINES); }
void vdbBeginPoints()                                { BeginImmediate(IMM_PRIM_POINTS); }

// convenience functions
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
    if (!render_texture::current_render_texture)
    {
        immediate::clear_color_was_set = true;
        immediate::clear_color = vdbVec4(r,g,b,a);
    }
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
