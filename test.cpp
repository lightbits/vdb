// HOW TO COMPILE
//   1) Obtain SDL 2 (https://www.libsdl.org/)
//
// WINDOWS
//   2) In your command line or batch script or whatever
//        cl test.cpp -MD -I"C:/path/to/sdl/include" /link -LIBPATH:"C:/path/to/sdl/lib/x86" SDL2.lib SDL2main.lib opengl32.lib
//   3) Make sure SDL2.dll is in the executable directory
//
// LINUX
//   2) In your command line or batch script or whatever
//        g++ test.cpp -o test -lGL `sdl2-config --cflags --libs`
//
// OSX
// No clue: See if this page helps. https://wiki.libsdl.org/Installation
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

    vdb("test 3D", [](vdb_input Input)
    {
        static float rotate_x = 0.0f;
        static float rotate_y = 0.0f;
        static float rotate_z = 0.0f;
        static float translate_x = 0.0f;
        static float translate_y = 0.0f;
        static float translate_z = -2.0f;
        float fov = PI / 4.0f;
        float z_near = 0.1f;
        float z_far = 10.0f;
        mat4 projection = mat_perspective(fov, Input.WindowWidth, Input.WindowHeight, z_near, z_far);
        mat4 model = mat_scale(1.0f);
        mat4 view = mat_rotate_x(rotate_x) *
                    mat_rotate_y(rotate_y) *
                    mat_rotate_z(rotate_z) *
                    mat_translate(translate_x, translate_y, translate_z);

        glClearColor(0.3f, 0.35f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vdbView3D(model, view, projection);

        glLineWidth(4.0f);
        glBegin(GL_LINES);
        for (int i = 0; i <= 20; i++)
        {
            float a = -2.0f + 4.0f*i/20.0f;
            glColor4f(0.8f, 0.7f, 0.51f, 1.0f);
            glVertex3f(-2.0f, a, 0.0f);
            glVertex3f(+2.0f, a, 0.0f);

            glVertex3f(a, -2.0f, 0.0f);
            glVertex3f(a, +2.0f, 0.0f);
        }

        glColor4f(1.0f, 0.2f, 0.1f, 1.0f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(+0.5f, 0.5f, 0.5f);
        glEnd();

        ImGui::SliderAngle("Rotate X", &rotate_x);
        ImGui::SliderAngle("Rotate Y", &rotate_y);
        ImGui::SliderAngle("Rotate Z", &rotate_z);
        ImGui::SliderFloat("Translate X", &translate_x, -1.0f, +1.0f);
        ImGui::SliderFloat("Translate Y", &translate_y, -1.0f, +1.0f);
        ImGui::SliderFloat("Translate Z", &translate_z, -1.0f, +1.0f);
    });

    const int width = 129;
    const int height = 128;
    unsigned char data[width*height*3];
    for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++)
    {
        unsigned char xmod = x % 32;
        unsigned char ymod = y % 32;
        unsigned char r = 64+4*xmod;
        unsigned char g = 64+4*ymod;
        unsigned char b = xmod+ymod;
        data[(x+y*width)*3+0] = r;
        data[(x+y*width)*3+1] = g;
        data[(x+y*width)*3+2] = b;
    }

    // test step once

    GLuint texture = 0;
    vdb("test 2", [&](vdb_input Input)
    {
        glClearColor(0.8f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vdbOrtho(-1.0f, +1.0f, +1.0f, -1.0f);
        vdbImage(data, width, height, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
    });

    // test step over

    for (int x = 0; x < 10; x++)
    {
        vdb("test 3", [&](vdb_input Input)
        {
            float A1 = 0.54f;
            float A2 = 0.55f;
            float A3 = 0.56f;
            float B1 = 0.5f;
            float B2 = 0.5f;
            float B3 = 0.7f;
            float C1 = 0.5f;
            float C2 = 0.5f;
            float C3 = 0.5f;
            float D1 = 0.7f;
            float D2 = 0.8f;
            float D3 = 0.88f;
            float t = x / 10.0f;
            if (t > 1.0f) t = 1.0f;
            if (t < 0.0f) t = 0.0f;
            float r = A1 + B1 * sin(TWO_PI * (C1 * t + D1));
            float g = A2 + B2 * sin(TWO_PI * (C2 * t + D2));
            float b = A3 + B3 * sin(TWO_PI * (C3 * t + D3));
            float a = 1.0f;
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
