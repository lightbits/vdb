// HOW TO COMPILE
//   Obtain SDL 2 (https://www.libsdl.org/)
//   e.g. sudo apt-get install libsdl2-dev
//
//   Then, run the build script for your platform to generate a library version of vdb.
//   (make for MINGW/UNIX/OSX and build.bat for MSVC)
//
// WINDOWS
//   In your command line
//        cl test.cpp -I"C:\path\to\vdb\include" /MD /link /LIBPATH:"C:\path\to\vdb\lib" vdb.lib
//   Make sure SDL2.dll is in the executable directory
//
// LINUX
//   In your command line
//        g++ test.cpp -I"/path/to/vdb/include" -o test -L"/path/to/vdb/lib" -lvdb
//
// OSX
//   In your command line
//        g++ test.cpp -I"/path/to/vdb/include" -o test -L"/path/to/vdb/lib" -lvdb
//
// If you have problems with SDL2 see this page: https://wiki.libsdl.org/Installation
#include <stdlib.h>
#include <math.h>
#include <vdb.h>
#include <vdb/imgui.h>

int main(int, char **)
{
    VDBB("Hello VDB");
    {
        vdbClearColor(0.45f, 0.56f, 0.6f, 1.0f);
        ImGui::Text("The program has now stopped and is running the\ncontent inside the brackets at 60 fps.");
        ImGui::Text("Press F10 to continue.");
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
            vdbClearColor(c[i%8][0], c[i%8][1], c[i%8][2], 1.0f);

            ImGui::Text("Iteration: %d", i);
            ImGui::Text("It works with for loops.");
            ImGui::Text("F10 steps once.");
            ImGui::Text("F5 skips to the next window.");
        }
        VDBE();
    }

    VDBB("Hello OpenGL");
    {
        vdbClearColor(1.0f, 0.73f, 0.22f, 1.0f);
        vdbTriangles();
        vdbVertex(-0.5f, -0.5f); vdbColor(1.0f, 0.5f, 0.5f, 1.0f);
        vdbVertex(+0.5f, -0.5f); vdbColor(0.5f, 1.0f, 0.5f, 1.0f);
        vdbVertex(+0.0f, +0.5f); vdbColor(0.5f, 0.5f, 1.0f, 1.0f);
        vdbEnd();

        ImGui::Text("Inside the loop you can draw stuff with OpenGL.");
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
        vdbLoadImageUint8(0, data, width, height, 3);
        vdbDrawImage(0);
        ImGui::TextWrapped("You can access variables outside the scope.");
    }
    VDBE();

    VDBB("ImGui");
    {
        vdbClearColor(0.99f, 0.55f, 0.38f, 1.0f);
        ImGui::ShowDemoWindow();
        ImGui::TextWrapped("VDB includes ImGui: https://github.com/ocornut/imgui/");
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

        vdbOrtho(-1,+1,-1,+1);
        vdbPoints(8.0f);
        for (int i = 0; i < num_items; i++)
        {
            float x = items[i].x;
            float y = items[i].y;
            vdbColor(0.3f, 0.3f, 0.3f, 1.0f);
            vdbVertex(x, y);

            if (vdbIsMouseOver(x, y))
                ImGui::SetTooltip("Hovered point\nx = %.2f\ny = %.2f", x, y);
        }
        vdbEnd();

        {
            float x,y;
            int i = vdbGetMouseOverIndex(&x, &y);
            vdbPoints(16.0f);
            vdbColor(1.0f, 0.9f, 0.2f, 0.5f);
            vdbVertex(x, y);
            vdbEnd();

            if (i < num_items-1)
            {
                float x2 = items[i+1].x;
                float y2 = items[i+1].y;
                vdbLines(2.0f);
                vdbVertex(x, y); vdbVertex(x2, y2);
                vdbEnd();
            }
        }

        ImGui::TextWrapped("You can check if an item you just drew is hovered over by the mouse.");
    }
    VDBE();

    VDBB("3D");
    {
        static float t = 0.0f; t += 1.0f/60.0f;
        vdbDepthTest(true);
        vdbDepthWrite(true);
        vdbClearDepth(1.0f);

        vdbMatrixEulerXYZ(0,0,-3, -0.9f,0.0f,0.7f + 0.3f*vdbGetMousePosNDC().x);
        vdbPerspective(3.14f/4.0f, vdbGetWindowWidth(), vdbGetWindowHeight(), 0.1f, 10.0f);
        vdbPoints(6.0f);
        {
            int nx = 64;
            int ny = 64;
            for (int yi = 0; yi <= ny; yi++)
            for (int xi = 0; xi <= nx; xi++)
            {
                float xt = (float)xi/nx;
                float yt = (float)yi/ny;

                float h = sinf(6.0f*xt+t)*cosf(7.0f*yt+t);
                h += 0.25f*sinf(13.0f*xt+1.2f*t)*cosf(18.0f*yt+1.5f*t);

                float x = -1.0f + 2.0f*xt;
                float y = -1.0f + 2.0f*yt;
                float z = 0.2f*h;

                float c = 0.5f+z;
                vdbColor(c, 0.5f*c, 0.2f*c, 1.0f);
                vdbVertex(x, y, z);
            }
        }
        vdbEnd();
    }
    VDBE();

    VDBB("3D hover");
    {
        vdbDepthTest(true);
        vdbDepthWrite(true);
        vdbClearDepth(1.0f);
        vdbMatrixEulerXYZ(0,0,-3, -0.9f,0.0f,0.7f + 0.3f*vdbGetMousePosNDC().x);
        vdbPerspective(3.14f/4.0f, vdbGetWindowWidth(), vdbGetWindowHeight(), 0.1f, 10.0f);
        vdbLines(4.0f);
        vdbColor(1,1,1, 0.5f);
        vdbVertex(-1,-1,0); vdbVertex(+1,-1,0);
        vdbVertex(-1, 0,0); vdbVertex(+1, 0,0);
        vdbVertex(-1,+1,0); vdbVertex(+1,+1,0);
        vdbVertex(-1,+1,0); vdbVertex(-1,-1,0);
        vdbVertex( 0,+1,0); vdbVertex( 0,-1,0);
        vdbVertex(+1,+1,0); vdbVertex(+1,-1,0);
        vdbEnd();
        vdbPoints(8.0f);
        srand(128);
        for (int i = 0; i < 256; i++)
        {
            float x = -1.0f + 2.0f*(rand()%1000)/1000.0f;
            float y = -1.0f + 2.0f*(rand()%1000)/1000.0f;
            float z = -1.0f + 2.0f*(rand()%1000)/1000.0f;
            if (vdbIsMouseOver(x,y,z))
            {
                ImGui::SetTooltip("%d", i);
            }
            vdbColor(0.5f+0.5f*x,0.5f+0.5f*y,0.5f+0.5f*z,1.0f);
            vdbVertex(x,y,z);
        }
        vdbEnd();

        vdbPoints(16.0f);
        vdbColor(1.0f, 0.9f, 0.2f, 0.5f);
        float x, y, z;
        vdbGetMouseOverIndex(&x, &y, &z);
        vdbVertex(x, y, z);
        vdbEnd();

        ImGui::TextWrapped("Hovering also works in 3D");
    }
    VDBE();
    return 0;
}
