// see bottom of file
extern const char *point_shader_vs;
extern const char *point_shader_fs;

struct gl_point_buffer_t
{
    GLuint position;
    GLuint color;
    int num_points;
    bool allocated;
};
enum { vdb_max_point_buffers = 1024 };
static gl_point_buffer_t point_buffers[vdb_max_point_buffers];

void vdbLoadPoints(int slot, vdbVec3 *position, vdbVec4 *color, int num_points)
{
    SDL_assert(slot >= 0 && slot < vdb_max_point_buffers && "You are trying to load points beyond the available slots.");

    gl_point_buffer_t *buffer = &point_buffers[slot];
    buffer->num_points = num_points;
    if (!buffer->allocated)
    {
        buffer->allocated = true;
        glGenBuffers(1, &buffer->position);
        glGenBuffers(1, &buffer->color);
    }
    // todo: buffersubdata if already allocated?
    glBindBuffer(GL_ARRAY_BUFFER, buffer->position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*num_points, (float*)position, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*num_points, (float*)color, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void vdbDrawPoints(int slot, float point_size, int circle_segments)
{
    typedef void (*vertex_attrib_divisor_t)(GLuint, GLuint);
    static vertex_attrib_divisor_t VertexAttribDivisor = (vertex_attrib_divisor_t)SDL_GL_GetProcAddress("glVertexAttribDivisor");
    SDL_assert(VertexAttribDivisor && "Failed to dynamically load OpenGL function.");

    // todo: gl deprecation, replace with storing own matrix stack?
    float projection[4*4];
    float model_to_view[4*4];
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_MODELVIEW_MATRIX, model_to_view);

    static bool shader_loaded = false;
    static GLuint program = 0;
    static GLint attrib_in_position = 0;
    static GLint attrib_in_color = 0;
    static GLint attrib_instance_position = 0;
    static GLint attrib_instance_color = 0;
    // static GLint uniform_reflection = 0;
    static GLint uniform_point_size = 0;
    static GLint uniform_projection = 0;
    static GLint uniform_model_to_view = 0;
    if (!shader_loaded)
    {
        program = LoadShaderFromMemory(point_shader_vs, point_shader_fs);
        attrib_in_position = glGetAttribLocation(program, "in_position");
        attrib_in_color = glGetAttribLocation(program, "in_color");
        attrib_instance_position = glGetAttribLocation(program, "instance_position");
        attrib_instance_color = glGetAttribLocation(program, "instance_color");
        // uniform_reflection = glGetUniformLocation(program, "reflection");
        uniform_projection = glGetUniformLocation(program, "projection");
        uniform_model_to_view = glGetUniformLocation(program, "model_to_view");
        uniform_point_size = glGetUniformLocation(program, "point_size");
        shader_loaded = true;
    }

    // todo: do we need a vertex array object...?

    glUseProgram(program);
    glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, projection);
    glUniformMatrix4fv(uniform_model_to_view, 1, GL_FALSE, model_to_view);
    glUniform1f(uniform_point_size, point_size);

    #if POINT_SHADER_QUAD==1
    static const float quad_position[] = { -1,-1, 1,-1, 1,1, 1,1, -1,1, -1,-1 };
    static const float quad_color[] = { 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1 };
    glVertexAttribPointer(attrib_in_position, 2, GL_FLOAT, GL_FALSE, 0, quad_position);
    glEnableVertexAttribArray(attrib_in_position);
    glVertexAttribPointer(attrib_in_color, 4, GL_FLOAT, GL_FALSE, 0, quad_color);
    glEnableVertexAttribArray(attrib_in_color);
    #else // POINT_SHADER_VERTEX
    const int max_circle_segments = 128;
    if (circle_segments > max_circle_segments)
        circle_segments = max_circle_segments;
    static float circle[max_circle_segments*3*2];
    for (int i = 0; i < circle_segments; i++)
    {
        float t1 = 2.0f*3.1415926f*(0.125f + i/(float)(circle_segments));
        float t2 = 2.0f*3.1415926f*(0.125f + (i+1)/(float)(circle_segments));
        circle[6*i+0] = 0.0f;
        circle[6*i+1] = 0.0f;
        circle[6*i+2] = cosf(t1);
        circle[6*i+3] = sinf(t1);
        circle[6*i+4] = cosf(t2);
        circle[6*i+5] = sinf(t2);
    }
    glVertexAttribPointer(attrib_in_position, 2, GL_FLOAT, GL_FALSE, 0, circle);
    glEnableVertexAttribArray(attrib_in_position);
    #endif

    gl_point_buffer_t buffer = point_buffers[slot];
    glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
    glEnableVertexAttribArray(attrib_instance_position);
    glVertexAttribPointer(attrib_instance_position, 3, GL_FLOAT, GL_FALSE, 0, 0);
    VertexAttribDivisor(attrib_instance_position, 1);

    glBindBuffer(GL_ARRAY_BUFFER, buffer.color);
    glEnableVertexAttribArray(attrib_instance_color);
    glVertexAttribPointer(attrib_instance_color, 4, GL_FLOAT, GL_FALSE, 0, 0);
    VertexAttribDivisor(attrib_instance_color, 1);

    glDrawArraysInstanced(GL_TRIANGLES, 0, circle_segments*3, buffer.num_points);
    glDisableVertexAttribArray(attrib_in_position);
    glDisableVertexAttribArray(attrib_instance_position);
    glDisableVertexAttribArray(attrib_instance_color);
    VertexAttribDivisor(attrib_instance_position, 0);
    VertexAttribDivisor(attrib_instance_color, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

#ifdef VDB_POINT_SHADER_QUAD
//
// This shader can be used to render the points as pixel-perfect circles,
// using textured quads. This option might be slower than the vertex version
// especially if you restrict yourself to low vertex count.
//
#pragma once
#define SHADER(S) "#version 150\n" #S
const char *point_shader_vs = SHADER(
in vec2 in_position;
in vec4 in_color;
in vec3 instance_position;
in vec4 instance_color;
uniform mat4 projection;
uniform mat4 model_to_view;
uniform float point_size;
out vec4 vertex_color;
out vec2 quad_position;
void main()
{
    quad_position = in_position;
    vertex_color = instance_color*in_color;
    vec4 position = model_to_view*vec4(instance_position,1.0) + point_size*vec4(in_position,0.0,0.0);
    gl_Position = projection*position;
}
);

const char *point_shader_fs = SHADER(
in vec4 vertex_color;
in vec2 quad_position;
out vec4 fragment_color;
void main()
{
    fragment_color = vertex_color;

    // NO ALPHA BLENDING
    if (dot(quad_position,quad_position) > 1.0)
        discard;
}
);
#undef SHADER

#else // VDB_POINT_SHADER_VERTEX
//
// This shader can be used to render the points as colored meshes. This gives
// you MSAA for free (if enabled), but you end up with non-perfect circles.
// This option might be faster than the quad shader, especially for low
// vertex counts.
//
#pragma once
#define SHADER(S) "#version 150\n" #S
const char *point_shader_vs = SHADER(
in vec2 in_position;
in vec3 instance_position;
in vec4 instance_color;
uniform mat4 projection;
uniform mat4 model_to_view;
uniform float point_size;
// uniform float reflection; // I used this once to spin the point around every frame
                             // to make it look like a circle
out vec4 vertex_color;
out vec2 quad_position;
void main()
{
    quad_position = in_position;
    vertex_color = instance_color;
    vec4 position = model_to_view*vec4(instance_position,1.0) + point_size*vec4(in_position,0.0,0.0);
    // vec4 position = model_to_view*vec4(instance_position,1.0) + reflection*point_size*vec4(in_position,0.0,0.0);
    gl_Position = projection*position;
}
);

const char *point_shader_fs = SHADER(
in vec4 vertex_color;
in vec2 quad_position;
out vec4 fragment_color;
void main()
{
    fragment_color = vertex_color;
}
);
#undef SHADER
#endif
