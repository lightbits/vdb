// HOW TO COMPILE
//   1) Obtain SDL 2 (https://www.libsdl.org/)
//   e.g. sudo apt-get install libsdl2-dev
//
// WINDOWS
//   2) In your command line or batch script or whatever
//        cl test.cpp -MD -I"C:/path/to/sdl/include" /link -LIBPATH:"C:/path/to/sdl/lib/x86" -subsystem:console SDL2.lib SDL2main.lib opengl32.lib
//   3) Make sure SDL2.dll is in the executable directory
//
// LINUX
//   2) In your command line or batch script or whatever
//        g++ test.cpp -o test -lGL `sdl2-config --cflags --libs`
//
// OSX
//   2) In your command line or batch script or whatever
//        g++ test.cpp -o test -framework OpenGL `sdl2-config --cflags —libs`
//
// If you have problems with SDL2 see this page: https://wiki.libsdl.org/Installation
#include "src/vdb.h"

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
        vdbClear(0.45f, 0.56f, 0.6f, 1.0f);
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
        vdbSetTexture(0, data, width, height, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
        vdbDrawTexture(0);

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

    VDBB("Hovering");
    {
        struct item_t { float x,y; };
        const int num_items = 128;
        static item_t items[num_items];

        srand(12345);
        for (int i = 0; i < num_items; i++)
        {
            items[i].x = -1.0f + 2.0f*(rand() % 10000)/10000.0f;
            items[i].y = -1.0f + 2.0f*(rand() % 10000)/10000.0f;
        }

        vdb2D(-1, +1, -1, +1);
        glPoints(8.0f);
        for (int i = 0; i < num_items; i++)
        {
            float x = items[i].x;
            float y = items[i].y;
            glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
            glVertex2f(x, y);

            if (vdbIsPointHovered(x, y))
                SetTooltip("Hovered point\nx = %.2f\ny = %.2f", x, y);
        }
        glEnd();

        {
            float x,y;
            int i = vdbGetHoveredPoint(&x, &y);
            glPoints(16.0f);
            glColor4f(1.0f, 0.9f, 0.2f, 0.5f);
            glVertex2f(x, y);
            glEnd();

            if (i < num_items-1)
            {
                float x2 = items[i+1].x;
                float y2 = items[i+1].y;
                glLines(2.0f);
                glVertex2f(x, y); glVertex2f(x2, y2);
                glEnd();
            }
        }

        TextWrapped("You can check if an item you just drew is hovered over by the mouse.");
    }
    VDBE();

    VDBB("3D");
    {
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        vdb3D();
        glPoints(6.0f);
        {
            int nx = 64;
            int ny = 64;
            for (int yi = 0; yi <= ny; yi++)
            for (int xi = 0; xi <= nx; xi++)
            {
                float xt = (float)xi/nx;
                float yt = (float)yi/ny;
                float t = 0.25f*vdb_input.t;

                float h = sinf(6.0f*xt+t)*cosf(7.0f*yt+t);
                h += 0.25f*sinf(13.0f*xt+1.2f*t)*cosf(18.0f*yt+1.5f*t);

                float x = -1.0f + 2.0f*xt;
                float y = -1.0f + 2.0f*yt;
                float z = 0.2f*h;

                float c = 0.5f+z;
                glColor4f(c, 0.5f*c, 0.2f*c, 1.0f);
                glVertex3f(x, y, z);
            }
        }
        glEnd();

        Text("VDB has a 3D camera!");
        Text("Press the arrow keys to move the camera");
        Text("Press z and x to zoom in and out");
        Text("Hold shift for fast movement");
    }
    VDBE();

    VDBB("3D hover");
    {
        vdb3D();
        glLines(4.0f);
        glColor4f(1,1,1, 0.5f);
        vdbGridXY(-1, +1, -1, +1, 2);
        glEnd();
        glPoints(8.0f);
        srand(128);
        for (int i = 0; i < 256; i++)
        {
            float x = -1.0f + 2.0f*(rand()%1000)/1000.0f;
            float y = -1.0f + 2.0f*(rand()%1000)/1000.0f;
            float z = -1.0f + 2.0f*(rand()%1000)/1000.0f;
            if (vdbIsPointHovered(x,y,z))
            {
                SetTooltip("%d", i);
            }
            glColor4f(0.5f+0.5f*x,0.5f+0.5f*y,0.5f+0.5f*z,1.0f);
            glVertex3f(x,y,z);
        }
        glEnd();

        glPoints(16.0f);
        glColor4f(1.0f, 0.9f, 0.2f, 0.5f);
        float x, y, z;
        vdbGetHoveredPoint(&x, &y, &z);
        glVertex3f(x, y, z);
        glEnd();

        TextWrapped("Hovering also works in 3D");
    }
    VDBE();

    return 0;
}
