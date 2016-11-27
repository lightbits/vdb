#include <stdlib.h>
#include <assert.h>
#ifndef sob_assert
#define sob_assert assert
#endif
#define sob_countof(X) (sizeof(X)/sizeof((X)[0]))

struct sob_vertex
{
    float size;
    float x, y, z, w;
    float r, g, b, a;
};

enum sob_mode
{
    sob_mode_line = 0,
    sob_mode_point = 1,
    sob_mode_triangle = 2
};

struct sob_draw_cmd
{
    sob_mode mode;
    sob_vertex v1;
    sob_vertex v2;
    sob_vertex v3;
    float size;
};

struct sob_draw_cmds
{
    sob_draw_cmd cmds[4096];
    int count;
};

static sob_draw_cmds _sob_draw_cmds;

int sob_sort_cmp(const void *a, const void *b)
{
    sob_draw_cmd ca = *(sob_draw_cmd*)a;
    sob_draw_cmd cb = *(sob_draw_cmd*)b;
    if (ca.mode < cb.mode)
    {
        return -1;
    }
    else if (ca.mode > cb.mode)
    {
        return +1;
    }
    else if (ca.mode == sob_mode_line || ca.mode == sob_mode_point)
    {
        if (ca.size < cb.size)
            return -1;
        else if (ca.size > cb.size)
            return +1;
        else
            return 0;
    }
    else
    {
        return 0;
    }
}

void sob_draw()
{
    if (_sob_draw_cmds.count == 0)
        return;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    qsort(_sob_draw_cmds.cmds, _sob_draw_cmds.count, sizeof(sob_draw_cmd), sob_sort_cmp);
    float size = 0.0f;
    int mode = -1;
    for (int i = 0; i < _sob_draw_cmds.count; i++)
    {
        sob_draw_cmd cmd = _sob_draw_cmds.cmds[i];
        if (cmd.mode != mode || cmd.size != size)
        {
            if (mode != -1)
            {
                glEnd();
            }

            mode = cmd.mode;
            size = cmd.size;

            if (mode == sob_mode_line)
            {
                glLineWidth(size);
                glBegin(GL_LINES);
            }
            if (mode == sob_mode_point)
            {
                glPointSize(size);
                glBegin(GL_POINTS);
            }
            if (mode == sob_mode_triangle)
            {
                glBegin(GL_TRIANGLES);
            }
        }
        if (mode == sob_mode_line)
        {
            glColor4f(cmd.v1.r, cmd.v1.g, cmd.v1.b, cmd.v1.a);
            glVertex4f(cmd.v1.x, cmd.v1.y, cmd.v1.z, cmd.v1.w);
            glColor4f(cmd.v2.r, cmd.v2.g, cmd.v2.b, cmd.v2.a);
            glVertex4f(cmd.v2.x, cmd.v2.y, cmd.v2.z, cmd.v2.w);
        }
        if (mode == sob_mode_point)
        {
            glColor4f(cmd.v1.r, cmd.v1.g, cmd.v1.b, cmd.v1.a);
            glVertex4f(cmd.v1.x, cmd.v1.y, cmd.v1.z, cmd.v1.w);
        }
        if (mode == sob_mode_triangle)
        {
            glColor4f(cmd.v1.r, cmd.v1.g, cmd.v1.b, cmd.v1.a);
            glVertex4f(cmd.v1.x, cmd.v1.y, cmd.v1.z, cmd.v1.w);
            glColor4f(cmd.v2.r, cmd.v2.g, cmd.v2.b, cmd.v2.a);
            glVertex4f(cmd.v2.x, cmd.v2.y, cmd.v2.z, cmd.v2.w);
            glColor4f(cmd.v3.r, cmd.v3.g, cmd.v3.b, cmd.v3.a);
            glVertex4f(cmd.v3.x, cmd.v3.y, cmd.v3.z, cmd.v3.w);
        }
    }
    glEnd();

    _sob_draw_cmds.count = 0;
}

void sob_triangle(sob_vertex v1, sob_vertex v2, sob_vertex v3)
{
    if (_sob_draw_cmds.count + 3 > sob_countof(_sob_draw_cmds.cmds))
    {
        sob_draw();
    }
    sob_assert(_sob_draw_cmds.count + 3 <= sob_countof(_sob_draw_cmds.cmds));
    sob_draw_cmd cmd;
    cmd.v1 = v1;
    cmd.v2 = v2;
    cmd.v3 = v3;
    cmd.mode = sob_mode_triangle;
    cmd.size = 1.0f;
    _sob_draw_cmds.cmds[_sob_draw_cmds.count++] = cmd;
}

void sob_line(sob_vertex v1, sob_vertex v2, float width)
{
    if (_sob_draw_cmds.count + 2 > sob_countof(_sob_draw_cmds.cmds))
    {
        sob_draw();
    }
    sob_assert(_sob_draw_cmds.count + 2 <= sob_countof(_sob_draw_cmds.cmds));
    sob_draw_cmd cmd;
    cmd.v1 = v1;
    cmd.v2 = v2;
    cmd.mode = sob_mode_line;
    cmd.size = width;
    _sob_draw_cmds.cmds[_sob_draw_cmds.count++] = cmd;
}

void sob_point(sob_vertex v, float size)
{
    if (_sob_draw_cmds.count + 1 > sob_countof(_sob_draw_cmds.cmds))
    {
        sob_draw();
    }
    sob_assert(_sob_draw_cmds.count + 1 <= sob_countof(_sob_draw_cmds.cmds));
    sob_draw_cmd cmd;
    cmd.v1 = v;
    cmd.mode = sob_mode_point;
    cmd.size = size;
    _sob_draw_cmds.cmds[_sob_draw_cmds.count++] = cmd;
}

#if 0
int sob_sort_cmp(const void *a, const void *b)
{
    sob_vertex va = *(sob_vertex*)a;
    sob_vertex vb = *(sob_vertex*)b;
    if      (va.key < vb.key) return -1;
    else if (va.key > vb.key) return +1;
    else                      return 0;
}

void sob_draw()
{
    if (_sob_state.count == 0)
        return;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    qsort(_sob_state.vertices, _sob_state.count, sizeof(sob_vertex), sob_sort_cmp);
    int key = -1;
    for (int i = 0; i < _sob_state.count; i++)
    {
        sob_vertex v = _sob_state.vertices[i];
        if (v.key != key)
        {
            if (key != -1)
            {
                ImGui::Text("glEnd");
                glEnd();
            }
            key = v.key;
            if (v.mode == sob_mode_line)
            {
                ImGui::Text("glBegin(GL_LINES)");
                glLineWidth(v.size);
                glBegin(GL_LINES);
            }
            if (v.mode == sob_mode_point)
            {
                ImGui::Text("glBegin(GL_POINTS)");
                glPointSize(v.size);
                glBegin(GL_POINTS);
            }
            if (v.mode == sob_mode_triangle)
            {
                glBegin(GL_TRIANGLES);
            }
        }
        ImGui::Text("%d %d %d: %.2f %.2f", i, v.key, v.mode, v.x, v.y);
        glColor4f(v.r, v.g, v.b, v.a);
        glVertex4f(v.x, v.y, v.z, v.w);
    }
    ImGui::Text("glEnd");
    glEnd();

    _sob_state.count = 0;
}

void sob_color(float r, float g, float b, float a=1.0f)
{
    _sob_state.color.r = r;
    _sob_state.color.g = g;
    _sob_state.color.b = b;
    _sob_state.color.a = a;
}

int sob_key(int mode, float size)
{
    if (size > 64.0f)
        size = 64.0f;
    int size_quantized = (int)((size/64.0f)*1023.0f);
    if (size_quantized > 1023)
        size_quantized = 1023;
    int key = mode*1024 + size_quantized;
    return key;
}

void sob_point(float x, float y, float z, float w,
               float size)
{
    if (_sob_state.count + 1 > sob_countof(_sob_state.vertices))
    {
        sob_draw();
    }
    sob_assert(_sob_state.count + 1 <= sob_countof(_sob_state.vertices));
    sob_vertex v1 = {0};
    v1.x = x;
    v1.y = y;
    v1.z = z;
    v1.w = w;
    v1.r = _sob_state.color.r;
    v1.g = _sob_state.color.g;
    v1.b = _sob_state.color.b;
    v1.a = _sob_state.color.a;
    v1.mode = sob_mode_point;
    v1.size = size;
    v1.key = sob_key(sob_mode_point, size);
    _sob_state.vertices[_sob_state.count++] = v1;
}

void sob_line(float x1, float y1, float z1, float w1,
              float x2, float y2, float z2, float w2,
              float width)
{
    if (_sob_state.count + 2 > sob_countof(_sob_state.vertices))
    {
        sob_draw();
    }
    sob_assert(_sob_state.count + 2 <= sob_countof(_sob_state.vertices));
    float x[2] = { x1, x2 };
    float y[2] = { y1, y2 };
    float z[2] = { z1, z2 };
    float w[2] = { w1, w2 };
    for (int i = 0; i < 2; i++)
    {
        sob_vertex v = {0};
        v.x = x[i];
        v.y = y[i];
        v.z = z[i];
        v.w = w[i];
        v.r = _sob_state.color.r;
        v.g = _sob_state.color.g;
        v.b = _sob_state.color.b;
        v.a = _sob_state.color.a;
        v.mode = sob_mode_line;
        v.size = width;
        v.key = sob_key(sob_mode_line, width);
        _sob_state.vertices[_sob_state.count++] = v;
    }
}

void sob_triangle(float x1, float y1, float z1, float w1,
                  float x2, float y2, float z2, float w2,
                  float x3, float y3, float z3, float w3)
{
    if (_sob_state.count + 3 > sob_countof(_sob_state.vertices))
    {
        sob_draw();
    }
    sob_assert(_sob_state.count + 3 <= sob_countof(_sob_state.vertices));
    float x[3] = { x1, x2, x3 };
    float y[3] = { y1, y2, y3 };
    float z[3] = { z1, z2, z3 };
    float w[3] = { w1, w2, w3 };
    for (int i = 0; i < 3; i++)
    {
        sob_vertex v = {0};
        v.x = x[i];
        v.y = y[i];
        v.z = z[i];
        v.w = w[i];
        v.r = _sob_state.color.r;
        v.g = _sob_state.color.g;
        v.b = _sob_state.color.b;
        v.a = _sob_state.color.a;
        v.mode = sob_mode_triangle;
        v.size = 1.0f;
        v.key = sob_key(sob_mode_triangle, 1.0f);
        _sob_state.vertices[_sob_state.count++] = v;
    }
}
#endif
