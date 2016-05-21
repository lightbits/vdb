#include "vdb.cpp"

int main(int argc, char **argv)
{
    vdb("test 1", [](vdb_input Input)
    {
        glClearColor(0.3f, 0.35f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.5f, -0.5f); glColor4f(1.0f, 0.5f, 0.5f, 1.0f);
        glVertex2f(+0.5f, -0.5f); glColor4f(0.5f, 1.0f, 0.5f, 1.0f);
        glVertex2f(+0.0f, +0.5f); glColor4f(0.5f, 0.5f, 1.0f, 1.0f);
        glEnd();

        ImGui::ShowTestWindow();
    });

    u08 data[128*128*3];
    for (int y = 0; y < 128; y++)
    for (int x = 0; x < 128; x++)
    {
        u08 xmod = x % 32;
        u08 ymod = y % 32;
        u08 r = 64+4*xmod;
        u08 g = 64+4*ymod;
        u08 b = xmod+ymod;
        data[(x+y*128)*3+0] = r;
        data[(x+y*128)*3+1] = g;
        data[(x+y*128)*3+2] = b;
    }

    // test step once

    GLuint texture = 0;
    vdb("test 2", [&](vdb_input Input)
    {
        glClearColor(0.8f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vdbOrtho(-1.0f, +1.0f, +1.0f, -1.0f);
        vdbImage(data, 128, 128, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
    });

    // test step over

    for (int x = 0; x < 10; x++)
    {
        vdb("test 3", [&](vdb_input Input)
        {
            r32 A1 = 0.54f;
            r32 A2 = 0.55f;
            r32 A3 = 0.56f;
            r32 B1 = 0.5f;
            r32 B2 = 0.5f;
            r32 B3 = 0.7f;
            r32 C1 = 0.5f;
            r32 C2 = 0.5f;
            r32 C3 = 0.5f;
            r32 D1 = 0.7f;
            r32 D2 = 0.8f;
            r32 D3 = 0.88f;
            r32 t = x / 10.0f;
            if (t > 1.0f) t = 1.0f;
            if (t < 0.0f) t = 0.0f;
            r32 r = A1 + B1 * sin(TWO_PI * (C1 * t + D1));
            r32 g = A2 + B2 * sin(TWO_PI * (C2 * t + D2));
            r32 b = A3 + B3 * sin(TWO_PI * (C3 * t + D3));
            r32 a = 1.0f;
            glClearColor(r, g, b, a);
            glClear(GL_COLOR_BUFFER_BIT);
        });
    }

    vdb("test 4", [](vdb_input Input)
    {
        glClearColor(0.8f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    });

    return 0;
}
