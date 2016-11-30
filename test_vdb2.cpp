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
#include "vdb2.h"

int main(int, char **)
{
    // Controls
    // F10 : Step once
    // F5 : Step over
    // Ctrl+V : Record video
    // Ctrl+R : Show ruler
    // Ctrl+W : Set window size
    // Escape : Close window

    VDBB("Hello VDB");
    {
        vdbClear(1.0f, 0.73f, 0.22f, 1.0f);
        Text("The program has now stopped and is running the\ncontent inside the brackets at 60 fps.");
        Text("Press F10 to continue.");
    }
    VDBE();

    for (int i = 0; i < 50; i++)
    {
        VDBB("For loops");
        {
            float c[][3] = {
                { 0.40, 0.76, 0.64 },
                { 0.99, 0.55, 0.38 },
                { 0.54, 0.63, 0.82 },
                { 0.91, 0.54, 0.77 },
                { 0.64, 0.86, 0.29 },
                { 1.00, 0.85, 0.19 },
                { 0.89, 0.77, 0.58 },
                { 0.70, 0.70, 0.70 },
            };
            glClearColor(c[i%8][0], c[i%8][1], c[i%8][2], 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            Text("Iteration: %d", i);
            Text("It works with for loops.");
            Text("F10 steps once.");
            Text("F5 skips to the next window.");
        }
        VDBE();
    }

    VDBB("Hello OpenGL");
    {
        glClearColor(1.0f, 0.73f, 0.22f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.5f, -0.5f); glColor4f(1.0f, 0.5f, 0.5f, 1.0f);
        glVertex2f(+0.5f, -0.5f); glColor4f(0.5f, 1.0f, 0.5f, 1.0f);
        glVertex2f(+0.0f, +0.5f); glColor4f(0.5f, 0.5f, 1.0f, 1.0f);
        glEnd();

        Text("Inside the loop you can draw stuff with OpenGL.");
    }
    VDBE();

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

    VDBB("Variable access");
    {
        vdbSetTexture2D(0, data, width, height, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
        vdbDrawTexture2D(0);

        TextWrapped("You can access variables outside the scope.");
    }
    VDBE();

    VDBB("ImGui");
    {
        vdbClear(0.99f, 0.55f, 0.38f, 1.0f);
        ShowTestWindow();

        TextWrapped("VDB includes ImGui: https://github.com/ocornut/imgui/");
    }
    VDBE();

    VDBB("Full bananacakes");
    {
        vdbFreeSphereCamera(3.1415f/4.0f, 0.01f, 100.0f);
        glBegin(GL_TRIANGLES);
        {
            int nx = (int)(vdb_input.width/64.0f);
            int ny = (int)(vdb_input.height/64.0f);
            vdb_for(yi, 0, ny+1)
            vdb_for(xi, 0, nx+1)
            {
                float xt = (float)xi/nx;
                float yt = (float)yi/ny;
                float xn = -1.0f + 2.0f*xt;
                float yn = -1.0f + 2.0f*yt;
                float dx = 2.0f/nx;
                float dy = 2.0f/ny;
                glColor4f(0.2f+0.8f*xt, 0.3f+0.7f*yt, 0.5f+0.5f*sinf(0.3f*vdb_input.t), 1.0f);
                glVertex2f(xn, yn);
                glVertex2f(xn+dx, yn);
                glVertex2f(xn+dx, yn+dy);
                glVertex2f(xn+dx, yn+dy);
                glVertex2f(xn, yn+dy);
                glVertex2f(xn, yn);

                if (xi == 7 && yi == 7)
                    vdbNote(xn+1.0f/nx, yn+1.0f/ny, "My coordinates are: %d %d", xi, yi);
                if (vdbMap(xn+1.0f/nx, yn+1.0f/ny))
                {
                    SetTooltip("%.2f %.2f", xn+1.0f/nx, yn+1.0f/ny);
                }
            }
        }
        glEnd();

        {
            int i;
            float x_src, y_src;
            vdbUnmap(&i, &x_src, &y_src);
            // SetTooltip("%d: %.2f %.2f", i, x_src, y_src);

            float x_win, y_win;
            vdbModelToWindow(x_src, y_src, 0.0f, 1.0f, &x_win, &y_win);
            vdbOrtho(0.0f, vdb_input.width, vdb_input.height, 0.0f);
            glBegin(GL_TRIANGLES);
            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            vdbFillCircle(x_win+1.0f, y_win+1.0f, 5.0f);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            vdbFillCircle(x_win, y_win, 5.0f);
            glEnd();
        }

        Text("Press the arrow keys to move the camera");
        Text("Press z and x to zoom in and out");
        Text("Hold shift for fast movement");
    }
    VDBE();

    VDBB("Ho");
    {
        vdbOrtho(-1.0f, +1.0f, -1.0f, +1.0f);
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        {
            mat3 R = m_mat3(mat_rotate_z(0.4f)*mat_rotate_y(0.4f)*mat_rotate_x(0.4f));
            int n = 16;
            vdb_for(zi, 0, n+1)
            vdb_for(yi, 0, n+1)
            vdb_for(xi, 0, n+1)
            {
                float xt = (float)xi/n;
                float yt = (float)yi/n;
                float zt = (float)zi/n;
                int it = xi*n*n+yi*n+zi;
                float offx = sinf(3.0f*vdb_input.t+0.0f+it)*(-1.0f+2.0f*noise_hash1f(it))/(4.0f*n);
                float offy = sinf(3.0f*vdb_input.t+1.5f+it)*(-1.0f+2.0f*noise_hash1f(it))/(4.0f*n);
                float offz = sinf(3.0f*vdb_input.t+3.1f+it)*(-1.0f+2.0f*noise_hash1f(it))/(4.0f*n);
                vec3 pm = m_normalize(m_vec3(-1.0f+2.0f*xt, -1.0f+2.0f*yt, -1.0f+2.0f*zt));
                pm += m_vec3(offx, offy, offz);
                vec3 pc = R*pm + m_vec3(0.0f, 0.0f, -5.0f);
                float f = (vdb_input.height/2.0f) / tanf(SO_PI/8.0f);
                vec2 uv = m_project_pinhole(f, f, vdb_input.width/2.0f, vdb_input.height/2.0f, pc);
                float xn = -1.0f + 2.0f*uv.x/vdb_input.width;
                float yn = -1.0f + 2.0f*uv.y/vdb_input.height;
                glColor4f(0.9f+0.1f*xt, 0.8f+0.2f*yt, 0.85f+0.15f*zt, 1.0f);
                glVertex2f(xn, yn);

                vdbMap(xn, yn);
            }
        }
        glEnd();

        {
            int i;
            float x_src, y_src;
            vdbUnmap(&i, &x_src, &y_src);
            SetTooltip("%d: %.2f %.2f", i, x_src, y_src);

            float x_win, y_win;
            vdbModelToWindow(x_src, y_src, 0.0f, 1.0f, &x_win, &y_win);
            vdbOrtho(0.0f, vdb_input.width, vdb_input.height, 0.0f);
            glBegin(GL_TRIANGLES);
            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            vdbFillCircle(x_win+1.0f, y_win+1.0f, 5.0f);
            glColor4f(1.0f, 1.0f, 0.3f, 1.0f);
            vdbFillCircle(x_win, y_win, 5.0f);
            glEnd();
        }
    }
    VDBE();

    return 0;
}
