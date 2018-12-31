#if defined(VDB_POINT_SHADER_QUAD)
#include "_point_shader_quad.cpp"
#elif defined(VDB_POINT_SHADER_VERTEX)
#include "_point_shader_vertex.cpp"
#else
#error "You must #define either VDB_POINT_SHADER_QUAD or VDB_POINT_SHADER_VERTEX."
#endif

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
    assert(slot >= 0 && slot < vdb_max_point_buffers && "You are trying to load points beyond the available slots.");

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

void vdbLoadPoints(int slot, void (*vertex_getter)(vdbVec3 *pos, vdbVec4 *col, void *data, int idx), void *data, int num_points)
{
    vdbVec3 *position = (vdbVec3*)malloc(num_points*sizeof(vdbVec3));
    vdbVec4 *color = (vdbVec4*)malloc(num_points*sizeof(vdbVec4));
    for (int idx = 0; idx < num_points; idx++)
    {
        color[idx].x = 1.0f;
        color[idx].y = 1.0f;
        color[idx].z = 1.0f;
        color[idx].w = 1.0f;
        vertex_getter(position + idx, color + idx, data, idx);
    }
    vdbLoadPoints(slot, position, color, num_points);
    free(position);
    free(color);
}

void vdbDrawPoints(int slot, float point_size, int circle_segments)
{
    assert(slot >= 0 && slot < vdb_max_point_buffers && "You're trying to draw a point cloud beyond the available slots.");
    if (!VertexAttribDivisor)
        VertexAttribDivisor = (GLVERTEXATTRIBDIVISORPROC)SDL_GL_GetProcAddress("glVertexAttribDivisor");
    assert(VertexAttribDivisor && "Failed to dynamically load OpenGL function.");

    static bool shader_loaded = false;
    static GLuint program = 0;
    static GLint attrib_in_position = 0;
    #if defined(VDB_POINT_SHADER_QUAD)
    static GLint attrib_in_color = 0;
    #endif
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
        #if defined(VDB_POINT_SHADER_QUAD)
        attrib_in_color = glGetAttribLocation(program, "in_color");
        #endif
        attrib_instance_position = glGetAttribLocation(program, "instance_position");
        attrib_instance_color = glGetAttribLocation(program, "instance_color");
        // uniform_reflection = glGetUniformLocation(program, "reflection");
        uniform_projection = glGetUniformLocation(program, "projection");
        uniform_model_to_view = glGetUniformLocation(program, "model_to_view");
        uniform_point_size = glGetUniformLocation(program, "point_size");
        shader_loaded = true;
    }

    // todo: vertex array object

    glUseProgram(program);
    #if defined(VDB_MATRIX_ROW_MAJOR)
    glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, transform::projection.data);
    glUniformMatrix4fv(uniform_model_to_view, 1, GL_FALSE, transform::view_model.data);
    #elif defined(VDB_MATRIX_COLUMN_MAJOR)
    glUniformMatrix4fv(uniform_projection, 1, GL_TRUE, transform::projection.data);
    glUniformMatrix4fv(uniform_model_to_view, 1, GL_TRUE, transform::view_model.data);
    #else
    #error "You must #define VDB_MATRIX_ROW_MAJOR or VDB_MATRIX_COLUMN_MAJOR"
    #endif
    glUniform1f(uniform_point_size, point_size);

    #if defined(VDB_POINT_SHADER_QUAD)

    static const float quad_position[] = { -1,-1, 1,-1, 1,1, 1,1, -1,1, -1,-1 };
    static const float quad_color[] = { 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1 };
    glVertexAttribPointer(attrib_in_position, 2, GL_FLOAT, GL_FALSE, 0, quad_position);
    glEnableVertexAttribArray(attrib_in_position);
    glVertexAttribPointer(attrib_in_color, 4, GL_FLOAT, GL_FALSE, 0, quad_color);
    glEnableVertexAttribArray(attrib_in_color);

    #elif defined(VDB_POINT_SHADER_VERTEX)

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

    #else
    #error "You must define either VDB_POINT_SHADER_VERTEX or VDB_POINT_SHADER_QUAD"
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
