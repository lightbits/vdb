// HOW TO COMPILE
//   1) Obtain SDL 2 (https://www.libsdl.org/)
//
// WINDOWS
//   2) In your command line or batch script or whatever
//        cl test.cpp -MD -I"C:/path/to/sdl/include" /link -LIBPATH:"C:/path/to/sdl/lib/x86" -subsystem:console SDL2.lib SDL2main.lib opengl32.lib
//   3) Make sure SDL2.dll is in the executable directory
//
// LINUX
//   2) In your command line or batch script or whatever
//        g++ test.cpp -std=c++11 -o test -lGL `sdl2-config --cflags --libs`
//
// OSX
//   2) In your command line or batch script or whatever
//        g++ test.cpp -std=c++11 -o test -framework OpenGL `sdl2-config --cflags —libs`
//
// If you have problems:
// No clue: See if this page helps. https://wiki.libsdl.org/Installation
#include "vdb.cpp"

int main(int argc, char **argv)
{
    vdb("step once", [](vdb_input Input)
    {
        ImGui::Begin("Information##1");
        ImGui::Text("The program has now stopped, and is running the content inside the brackets at 60 fps.");
        ImGui::Text("Press F10 to continue, or move your mouse to the top and click Debugger->Step once.");
        ImGui::End();
    });

    vdb("test gui", [](vdb_input Input)
    {
        glClearColor(0.3f, 0.35f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.5f, -0.5f); glColor4f(1.0f, 0.5f, 0.5f, 1.0f);
        glVertex2f(+0.5f, -0.5f); glColor4f(0.5f, 1.0f, 0.5f, 1.0f);
        glVertex2f(+0.0f, +0.5f); glColor4f(0.5f, 0.5f, 1.0f, 1.0f);
        glEnd();

        ImGui::Begin("Information##2");
        ImGui::Text("Inside the loop you can draw stuff with OpenGL.");
        ImGui::End();
    });

    vdb("test gui", [](vdb_input Input)
    {
        ImGui::ShowTestWindow();

        ImGui::Begin("Information##3");
        ImGui::Text("vdb includes ImGui (https://github.com/ocornut/imgui/), a fantastic GUI library.");
        ImGui::End();
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

    // draw the texture!

    GLuint texture = 0;
    vdb("test 2", [&](vdb_input Input)
    {
        glClearColor(0.8f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vdbOrtho(-1.0f, +1.0f, +1.0f, -1.0f);
        vdbImage(data, width, height, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);

        ImGui::Begin("Information##4");
        ImGui::Text("This shows that you can access variables outside the loop scope.\nIn this case, we are drawing a texture we generated ourselves, directly to the window.");
        ImGui::End();
    });

    // breakpoints in a loop

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

            ImGui::Begin("Information##5");
            ImGui::Text("Loop iteration: %d", x);
            ImGui::Text("This illustrates breaking in a loop. You can step over the loop\nby pressing F5.");
            ImGui::End();
        });
    }

    vdb("test bananacakes", [](vdb_input Input)
    {
        // some colors
        #define BLACK  0.15f, 0.13f, 0.10f, 1.0f
        #define RED    1.00f, 0.20f, 0.10f, 1.0f
        #define GREEN  0.10f, 1.00f, 0.20f, 1.0f
        #define BLUE   0.15f, 0.35f, 0.85f, 1.0f
        #define WHITE  0.80f, 0.70f, 0.51f, 1.0f

        mat4 projection = mat_perspective(PI/4.0f, Input.WindowWidth, Input.WindowHeight, 0.01f, 20.0f);
        mat4 view = vdbCamera3D(Input);
        glEnable(GL_DEPTH_TEST);
        glDepthRange(0.0f, 1.0f);
        glDepthFunc(GL_LEQUAL);
        glClearColor(BLACK);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vdbView3D(m_id4(), view, projection);
        glLines(2.0f);
        glColor4f(WHITE);
        vdbGridXY(-2.0f, +2.0f, -2.0f, +2.0f, 20);
        glEnd();

        mat3 R = m_mat3(mat_rotate_z(0.3f)*mat_rotate_y(0.5f)*mat_rotate_x(0.3f));
        vec3 p = m_vec3(0.5f, -0.5f, 0.5f);

        vdbView3D(m_se3(R, p)*mat_scale(0.5f), view, projection);
        glLines(4.0f);
        glColor4f(RED);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        glColor4f(GREEN);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glColor4f(BLUE);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);
        glEnd();

        vdbView3D(m_id4(), view, projection);
        glLines(4.0f);

        glColor4f(RED);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        glColor4f(GREEN);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glColor4f(BLUE);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);

        glColor4f(WHITE);
        glLine3f(p, p - 0.6f*R.a3);
        glLine3f(p, p - m_dot(0.6f*R.a3,m_vec3(0.0f, 0.0f, 1.0f))*m_vec3(0.0f, 0.0f, 1.0f));
        glEnd();

        ImGui::Begin("Let's go full bananacakes!");
        ImGui::Text("Camera controls:\nArrow keys to rotate, z and x to zoom.\nHold shift for speedy movement.");
        ImGui::End();
    });

    return 0;
}
