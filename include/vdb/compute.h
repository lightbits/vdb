/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Tiny cross-platform GPGPU module
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note that this is based on fragment shaders. As such, there is
no fine-grained control over kernel execution, e.g. division
of work. On the flipside, we can run on any platform that has
OpenGL 3.1!

This can be useful for simple GPGPU prototypes which need to
run on multiple platforms, but may not be particularly fast.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Example usage
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Below is a simple program that adds two arrays X and Y and
stores the result in a third array Z. The compute module
can be run both inside a VDB block or outside a VDB block.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <vdb.h>
#include <vdb/imgui.h>
#include <vdb/compute.h>
int main()
{
    vdbKernel program = vdbLoadComputeKernel(R"STR(
        uniform sampler1D X;
        uniform sampler1D Y;
        uniform float pi;
        out float color;
        void main() {
            int i = int(gl_FragCoord.x);
            float x = texelFetch(X, i, 0).r;
            float y = texelFetch(Y, i, 0).r;
            color = x + y + pi;
        }
        )STR");

    int n = 4;

    float *X_cpu = new float[n];
    float *Y_cpu = new float[n];
    for (int i = 0; i < n; i++) X_cpu[i] = 10.0f*i;
    for (int i = 0; i < n; i++) Y_cpu[i] = 1.0f*i;

    vdbGPUArray *X = vdbCreateGPUArray1f(n, 1, 1, X_cpu); // input
    vdbGPUArray *Y = vdbCreateGPUArray1f(n, 1, 1, Y_cpu); // input
    vdbGPUArray *Z = vdbCreateGPUArray1f(n, 1, 1);        // output
    float *Z_cpu = new float[vdbGPUArraySize(Z)];
    vdbGPUArrayClear(Z);
    vdbGPUArrayToCPU(Z, Z_cpu);

    VDBB("hi");
    {
        if (ImGui::Button("Run kernel"))
        {
            vdbUseKernel(program);
            vdbKernelParamArray("X", X, 0);
            vdbKernelParamArray("Y", Y, 1);
            vdbKernelParam1f("pi", 3.1415926f);
            vdbRunKernel(Z);
            vdbUseKernel(0);
            vdbGPUArrayToCPU(Z, Z_cpu);
        }
        if (ImGui::Button("Clear array"))
        {
            vdbGPUArrayClear(Z);
            vdbGPUArrayToCPU(Z, Z_cpu);
        }
        for (size_t j = 0; j < vdbGPUArraySize(Z); j++)
            ImGui::Text("%6.2f", Z_cpu[j]);
    }
    VDBE();
}

*/

#pragma once
#include "opengl.h"
#include <assert.h>

typedef GLuint vdbKernel;
struct vdbGPUArray;

void         vdbGPUArrayToCPU(vdbGPUArray *a, void *cpu_memory);
void         vdbGPUArrayClear(vdbGPUArray *a);
size_t       vdbGPUArraySize(vdbGPUArray *a);
void         vdbGPUArrayShape(vdbGPUArray *a, int *width, int *height, int *depth);
vdbGPUArray *vdbCreateGPUArray1f (int width, int height, int depth, const void *data=NULL);
vdbGPUArray *vdbCreateGPUArray2f (int width, int height, int depth, const void *data=NULL);
vdbGPUArray *vdbCreateGPUArray4f (int width, int height, int depth, const void *data=NULL);
vdbGPUArray *vdbCreateGPUArray1ub(int width, int height, int depth, const void *data=NULL);
vdbGPUArray *vdbCreateGPUArray2ub(int width, int height, int depth, const void *data=NULL);
vdbGPUArray *vdbCreateGPUArray4ub(int width, int height, int depth, const void *data=NULL);
void         vdbDestroyGPUArray(vdbGPUArray *a);
vdbKernel    vdbLoadComputeKernel(const char *source);
void         vdbUseKernel(vdbKernel program);
void         vdbRunKernel(vdbGPUArray *out);
void         vdbKernelParamArray(const char *name, vdbGPUArray *v, int texture_unit);
void         vdbKernelParam1f(const char *name, float x);
void         vdbKernelParam2f(const char *name, float x, float y);
void         vdbKernelParam3f(const char *name, float x, float y, float z);
void         vdbKernelParam4f(const char *name, float x, float y, float z, float w);
void         vdbKernelParam1i(const char *name, int x);
void         vdbKernelParam2i(const char *name, int x, int y);
void         vdbKernelParam3i(const char *name, int x, int y, int z);
void         vdbKernelParam4i(const char *name, int x, int y, int z, int w);
void         vdbKernelParamMatrix4fv(const char *name, float *x);
void         vdbKernelParamMatrix3fv(const char *name, float *x);
void         vdbKernelParamMatrix4fv_RowMaj(const char *name, float *x);
void         vdbKernelParamMatrix3fv_RowMaj(const char *name, float *x);

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Implementation //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

struct vdbGPUArray
{
    GLuint fbo;
    GLuint color0;
    int width;
    int height;
    int depth;
    int channels;
    GLenum target;
    GLenum internal_format;
    GLenum data_format;
    GLenum data_type;
};

static vdbKernel vdb_active_kernel = 0;

void vdbGPUArrayEnsureWriteable(vdbGPUArray *a)
{
    vdbMakeContextCurrent();
    assert(glGetError() == GL_NO_ERROR);
    if (!a->fbo)
    {
        assert(a->color0);
        glGenFramebuffers(1, &a->fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, a->fbo);
        assert(a->target == GL_TEXTURE_1D || a->target == GL_TEXTURE_2D);
        if (a->target == GL_TEXTURE_1D)
            glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, a->target, a->color0, 0);
        else if (a->target == GL_TEXTURE_2D)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, a->target, a->color0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        assert(glGetError() == GL_NO_ERROR);
    }
}

vdbGPUArray *vdbCreateGPUArray(
    const void *data,
    int channels,
    int width,
    int height,
    int depth,
    GLenum internal_format,
    GLenum data_format,
    GLenum data_type)
{
    vdbMakeContextCurrent();
    assert(glGetError() == GL_NO_ERROR);
    assert(channels > 0 && channels <= 4);
    assert(width > 0 && height > 0 && depth > 0);
    assert(channels == 1 || channels == 2 || channels == 4);

    GLenum target;
    if (width > 1 && height > 1 && depth > 1)
        target = GL_TEXTURE_3D;
    else if (width > 1 && height > 1)
        target = GL_TEXTURE_2D;
    else
        target = GL_TEXTURE_1D;

    GLuint color0 = 0;
    {
        glGenTextures(1, &color0);
        glBindTexture(target, color0);
        if (target == GL_TEXTURE_1D)
        {
            glTexImage1D(target, 0, internal_format, width, 0, data_format, data_type, data);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        }
        else if (target == GL_TEXTURE_2D)
        {
            glTexImage2D(target, 0, internal_format, width, height, 0, data_format, data_type, data);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else if (target == GL_TEXTURE_3D)
        {
            glTexImage3D(target, 0, internal_format, width, height, depth, 0, data_format, data_type, data);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(target, 0);
        assert(glGetError() == GL_NO_ERROR);
    }

    vdbGPUArray *a = (vdbGPUArray*)calloc(1, sizeof(vdbGPUArray));
    a->color0 = color0;
    a->fbo = 0;
    a->channels = channels;
    a->width = width;
    a->height = height;
    a->depth = depth;
    a->internal_format = internal_format;
    a->data_format = data_format;
    a->data_type = data_type;
    a->target = target;
    if (!data)
        vdbGPUArrayEnsureWriteable(a);
    assert(glGetError() == GL_NO_ERROR);
    return a;
}


vdbGPUArray *vdbCreateGPUArray1f (int w, int h, int d, const void *x) { return vdbCreateGPUArray(x, 1, w, h, d, GL_R32F, GL_RED, GL_FLOAT); }
vdbGPUArray *vdbCreateGPUArray2f (int w, int h, int d, const void *x) { return vdbCreateGPUArray(x, 2, w, h, d, GL_RG32F, GL_RG, GL_FLOAT); }
vdbGPUArray *vdbCreateGPUArray4f (int w, int h, int d, const void *x) { return vdbCreateGPUArray(x, 4, w, h, d, GL_RGBA32F, GL_RGBA, GL_FLOAT); }
vdbGPUArray *vdbCreateGPUArray1ub(int w, int h, int d, const void *x) { return vdbCreateGPUArray(x, 1, w, h, d, GL_R8, GL_RED, GL_UNSIGNED_BYTE); }
vdbGPUArray *vdbCreateGPUArray2ub(int w, int h, int d, const void *x) { return vdbCreateGPUArray(x, 2, w, h, d, GL_RG8, GL_RG, GL_UNSIGNED_BYTE); }
vdbGPUArray *vdbCreateGPUArray4ub(int w, int h, int d, const void *x) { return vdbCreateGPUArray(x, 4, w, h, d, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE); }

void vdbDestroyGPUArray(vdbGPUArray *a)
{
    vdbMakeContextCurrent();
    assert(glGetError() == GL_NO_ERROR);
    if (a)
    {
        glDeleteTextures(1, &a->color0);
        glDeleteFramebuffers(1, &a->fbo);
        free(a);
        assert(glGetError() == GL_NO_ERROR);
    }
}

void vdbGPUArrayToCPU(vdbGPUArray *a, void *cpu_memory)
{
    vdbMakeContextCurrent();
    assert(glGetError() == GL_NO_ERROR);
    assert(cpu_memory);
    assert(a->color0);
    assert(glGetError() == GL_NO_ERROR);
    assert(a->target == GL_TEXTURE_1D || a->target == GL_TEXTURE_2D);
    assert(a->depth == 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glBindTexture(a->target, a->color0);
    glGetTexImage(a->target, 0, a->data_format, a->data_type, cpu_memory);
    glBindTexture(a->target, 0);
    assert(glGetError() == GL_NO_ERROR);
}

size_t vdbGPUArraySize(vdbGPUArray *a)
{
    return a->width*a->height*a->depth*a->channels;
}

void vdbGPUArrayClear(vdbGPUArray *a)
{
    vdbMakeContextCurrent();
    assert(glGetError() == GL_NO_ERROR);
    vdbGPUArrayEnsureWriteable(a);
    assert(a->fbo);
    assert(a->color0);
    GLint last_framebuffer; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, a->fbo);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, last_framebuffer);
    assert(glGetError() == GL_NO_ERROR);
}

static GLuint vdbCompileShader(GLenum type, const char **sources, int num_sources)
{
    assert(glGetError() == GL_NO_ERROR);
    GLuint shader = glCreateShader(type);
    assert(shader);
    glShaderSource(shader, num_sources, (const GLchar **)sources, 0);
    glCompileShader(shader);
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLint length; glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char *info = (char*)malloc(length);
        glGetShaderInfoLog(shader, length, NULL, info);
        printf("Failed to compile shader:\n%s", info);
        free(info);
        glDeleteShader(shader);
        return 0;
    }
    assert(glGetError() == GL_NO_ERROR);
    return shader;
}

static bool vdbProgramLinkStatus(GLuint program)
{
    assert(glGetError() == GL_NO_ERROR);
    GLint status; glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char *info = (char*)malloc(length);
        glGetProgramInfoLog(program, length, NULL, info);
        printf("Failed to link program:\n%s", info);
        free(info);
        return false;
    }
    assert(glGetError() == GL_NO_ERROR);
    return true;
}

vdbKernel vdbLoadComputeKernel(const char *source)
{
    vdbMakeContextCurrent();
    assert(glGetError() == GL_NO_ERROR);
    assert(source);
    const char *vs_source = R"STR(
        #version 150
        in vec2 iPosition;
        void main() {
            gl_Position = vec4(iPosition, 0.0, 1.0);
        }
        )STR";
    GLuint vs = vdbCompileShader(GL_VERTEX_SHADER, &vs_source, 1);

    const char *sources[] = {
        "#version 150\n"
        "\nuniform int Dummy;\n"
        "#define ZERO (min(0, Dummy))\n"
        "\n#line 0\n",
        (const char*)source,
    };
    int num_sources = sizeof(sources)/sizeof(sources[0]);
    GLuint fs = vdbCompileShader(GL_FRAGMENT_SHADER, sources, num_sources);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    assert(vdbProgramLinkStatus(program));
    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    assert(glGetError() == GL_NO_ERROR);
    return program;
}

void vdbUseKernel(vdbKernel program)
{
    vdbMakeContextCurrent();
    assert(glGetError() == GL_NO_ERROR);

    static GLint last_program;
    static GLint last_array_buffer;
    static GLint last_vertex_array;
    static GLint last_viewport[4];
    static GLint last_scissor_box[4];
    static GLint last_framebuffer;
    static GLenum last_blend_src_rgb;
    static GLenum last_blend_dst_rgb;
    static GLenum last_blend_src_alpha;
    static GLenum last_blend_dst_alpha;
    static GLenum last_blend_equation_rgb;
    static GLenum last_blend_equation_alpha;
    static GLboolean last_depth_writemask;
    static GLenum last_enable_blend;
    static GLenum last_enable_cull_face;
    static GLenum last_enable_depth_test;
    static GLenum last_enable_scissor_test;
    static GLenum last_enable_color_logic_op;
    static GLuint vao = 0;

    static GLuint quad = 0;
    if (!quad)
    {
        static const float data[] = { -1,-1, +1,-1, +1,+1, +1,+1, -1,+1, -1,-1 };
        glGenBuffers(1, &quad);
        glBindBuffer(GL_ARRAY_BUFFER, quad);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    assert(quad && "Failed to create vertex buffer.");

    if (vdb_active_kernel)
    {
        if (program)
        {
            assert(glIsProgram(program) && "Program must be a valid kernel object.");
            vdb_active_kernel = program;
            glUseProgram(program);
            GLint loc_iPosition = glGetAttribLocation(program, "iPosition");
            assert(loc_iPosition >= 0);
            glEnableVertexAttribArray(loc_iPosition);
            glVertexAttribPointer(loc_iPosition, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);
            assert(glGetError() == GL_NO_ERROR);
        }
        else
        {
            GLint loc_iPosition = glGetAttribLocation(vdb_active_kernel, "iPosition");
            glDisableVertexAttribArray(loc_iPosition);
            glDeleteVertexArrays(1, &vao);
            vdb_active_kernel = 0;

            // Restore GL state
            glUseProgram(last_program);
            glBindVertexArray(last_vertex_array);
            glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
            glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
            glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
            glBindFramebuffer(GL_FRAMEBUFFER, last_framebuffer);
            if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
            if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
            if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
            if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
            if (last_enable_color_logic_op) glEnable(GL_COLOR_LOGIC_OP); else glDisable(GL_COLOR_LOGIC_OP);
            glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
            glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
            glActiveTexture(GL_TEXTURE0);
            assert(glGetError() == GL_NO_ERROR);
        }
    }
    else
    {
        if (program)
        {
            assert(glIsProgram(program) && "Program must be a valid kernel object.");

            // Back-up GL state
            glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
            glGetIntegerv(GL_VIEWPORT, last_viewport);
            glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_framebuffer);
            glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
            glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
            glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
            glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
            glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
            glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
            glGetBooleanv(GL_DEPTH_WRITEMASK, (GLboolean*)&last_depth_writemask);
            last_enable_blend = glIsEnabled(GL_BLEND);
            last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
            last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
            last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
            last_enable_color_logic_op = glIsEnabled(GL_COLOR_LOGIC_OP);

            vdb_active_kernel = program;
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_SCISSOR_TEST);
            glDisable(GL_COLOR_LOGIC_OP);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, quad);

            glUseProgram(program);
            GLint loc_iPosition = glGetAttribLocation(program, "iPosition");
            assert(loc_iPosition >= 0);
            glEnableVertexAttribArray(loc_iPosition);
            glVertexAttribPointer(loc_iPosition, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);
            assert(glGetError() == GL_NO_ERROR);
        }
        else
        {
            // do nothing
        }
    }
    assert(glGetError() == GL_NO_ERROR);
}

void vdbRunKernel(vdbGPUArray *out)
{
    vdbMakeContextCurrent();
    assert(glGetError() == GL_NO_ERROR);
    assert(out);
    vdbGPUArrayEnsureWriteable(out);
    glBindFramebuffer(GL_FRAMEBUFFER, out->fbo);
    if (out->width > 1 && out->height > 1)
        glViewport(0, 0, out->width, out->height);
    else if (out->width > 1)
        glViewport(0, 0, out->width, 1);
    else
        assert(false);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    assert(glGetError() == GL_NO_ERROR);
}

void vdbKernelParamArray(const char *name, vdbGPUArray *v, int texture_unit)
{
    assert(vdb_active_kernel != 0);
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(v->target, v->color0);
    glUniform1i(glGetUniformLocation(vdb_active_kernel, name), texture_unit);
}

void vdbKernelParam1f(const char *name, float x)                            { assert(vdb_active_kernel != 0); glUniform1f(glGetUniformLocation(vdb_active_kernel, name), x); }
void vdbKernelParam2f(const char *name, float x, float y)                   { assert(vdb_active_kernel != 0); glUniform2f(glGetUniformLocation(vdb_active_kernel, name), x,y); }
void vdbKernelParam3f(const char *name, float x, float y, float z)          { assert(vdb_active_kernel != 0); glUniform3f(glGetUniformLocation(vdb_active_kernel, name), x,y,z); }
void vdbKernelParam4f(const char *name, float x, float y, float z, float w) { assert(vdb_active_kernel != 0); glUniform4f(glGetUniformLocation(vdb_active_kernel, name), x,y,z,w); }
void vdbKernelParam1i(const char *name, int x)                              { assert(vdb_active_kernel != 0); glUniform1i(glGetUniformLocation(vdb_active_kernel, name), x); }
void vdbKernelParam2i(const char *name, int x, int y)                       { assert(vdb_active_kernel != 0); glUniform2i(glGetUniformLocation(vdb_active_kernel, name), x,y); }
void vdbKernelParam3i(const char *name, int x, int y, int z)                { assert(vdb_active_kernel != 0); glUniform3i(glGetUniformLocation(vdb_active_kernel, name), x,y,z); }
void vdbKernelParam4i(const char *name, int x, int y, int z, int w)         { assert(vdb_active_kernel != 0); glUniform4i(glGetUniformLocation(vdb_active_kernel, name), x,y,z,w); }
void vdbKernelParamMatrix4fv(const char *name, float *x)                    { assert(vdb_active_kernel != 0); glUniformMatrix4fv(glGetUniformLocation(vdb_active_kernel, name), 1, false, x);}
void vdbKernelParamMatrix3fv(const char *name, float *x)                    { assert(vdb_active_kernel != 0); glUniformMatrix3fv(glGetUniformLocation(vdb_active_kernel, name), 1, false, x);}
void vdbKernelParamMatrix4fv_RowMaj(const char *name, float *x)             { assert(vdb_active_kernel != 0); glUniformMatrix4fv(glGetUniformLocation(vdb_active_kernel, name), 1, true, x);}
void vdbKernelParamMatrix3fv_RowMaj(const char *name, float *x)             { assert(vdb_active_kernel != 0); glUniformMatrix3fv(glGetUniformLocation(vdb_active_kernel, name), 1, true, x);}
