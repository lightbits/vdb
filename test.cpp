// HOW TO COMPILE
// You will need SDL2 (http://www.libsdl.org):
// Linux:
//   apt-get install libsdl2-dev
// Mac OS X:
//   brew install sdl2
// MSYS2:
//   pacman -S mingw-w64-i686-SDL
//
// Then, run the build script for your platform to generate a library version of vdb.
// (build.bat for Windows, build.sh for Linux and Mac OSX).
//
// WINDOWS
//   cl test.cpp -I"path\to\vdb\include" /MD /link /LIBPATH:"path\to\vdb\lib" vdb.lib
//   Ensure that SDL2.dll is in the executable directory
//
// On Linux and similar Unixes
//   g++ `sdl2-config --cflags` -Iinclude test.cpp -o test -Llib -lvdb `sdl2-config --libs` -lGL -ldl
//
// On Mac OSX
//   g++ `sdl2-config --cflags` -Iinclude test.cpp -o test -Llib -lvdb `sdl2-config --libs` -framework OpenGL -framework CoreFoundation
//
// If you have problems with SDL2 see this page: https://wiki.libsdl.org/Installation
#include <stdlib.h>
#include <math.h>
#include <vdb.h>
#include <vdb/imgui.h>

float frand()
{
    return (rand() % 1024) / 1024.0f;
}

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
                { 0.40f, 0.76f, 0.64f },
                { 0.99f, 0.55f, 0.38f },
                { 0.54f, 0.63f, 0.82f },
                { 0.91f, 0.54f, 0.77f },
                { 0.64f, 0.86f, 0.29f },
                { 1.00f, 0.85f, 0.19f },
                { 0.89f, 0.77f, 0.58f },
                { 0.70f, 0.70f, 0.70f },
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

        vdbCameraTurntable(vdbVec3(0,0,0), 1.0f);
        vdbPushMatrixEulerXYZ(0,0,0, -3.14f/2.0f,0,0);
        vdbPerspective(3.14f/4.0f, 0.1f, 10.0f);
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
        vdbPopMatrix();
    }
    VDBE();

    VDBB("3D hover");
    {
        vdbDepthTest(true);
        vdbDepthWrite(true);
        vdbClearDepth(1.0f);
        vdbMatrixEulerXYZ(0,0,-3, -0.9f,0.0f,0.7f + 0.3f*vdbGetMousePosNDC().x);
        vdbPerspective(3.14f/4.0f, 0.1f, 10.0f);
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

    VDBB("point clouds");
    {
        const int N = 8;
        const int num_points = N*N*N;
        static vdbVec3 position[num_points];
        static vdbVec4 color[num_points];
        float *x = (float*)position;
        if (vdbIsFirstFrame())
        {
            for (int i = 0; i < num_points; i++)
            {
                float r = (i % N)/(float)N;
                float g = ((i / N) % N)/(float)N;
                float b = ((i / N) / N)/(float)N;
                float x = -1.0f+2.0f*(r + 0.5f/N);
                float y = -1.0f+2.0f*(g + 0.5f/N);
                float z = -1.0f+2.0f*(b + 0.5f/N);
                position[i] = vdbVec3(x,y,z);
                color[i] = vdbVec4(r,g,0.5f+0.5f*b,1);
            }
            vdbLoadPoints(0, position, color, num_points);
        }
        // temporal super sampling
        #if 0
        {
            float dx,dy;
            vdbBeginTSS(160,120,2,&dx,&dy);
            vdbPerspective(3.14f/4.0f, 0.1f, 10.0f, dx, dy);
        }
        #else
        // temporal anti-aliasing (really just low-pass filtering)
        {
            vdbBeginTAA(0, 0.9f);
            float pixel_width = 2.0f/vdbGetFramebufferWidth();
            float pixel_height = 2.0f/vdbGetFramebufferHeight();
            float dx = pixel_width*(-1.0f + 2.0f*frand());
            float dy = pixel_height*(-1.0f + 2.0f*frand());
            vdbPerspective(3.14f/4.0f, 0.1f, 10.0f, dx, dy);
        }
        #endif
        static float t = 0.0f; t += 1.0f/60.0f;
        vdbMatrixEulerXYZ(0,0,-5, 0.3f,0.1f*t,0);
        vdbDepthTest(true);
        vdbDepthWrite(true);
        vdbClearDepth(1.0f);
        vdbClearColor(0,0,0,1);
        vdbDrawPoints(0, 0.1f, 32);
    }
    VDBE();

    VDBB("shader");
    {
        #define SHADER(S) "#version 150\n" #S
        const char *fs = SHADER(
        uniform vec2 Resolution;
        out vec4 Color;

        void main()
        {
            vec2 uv = gl_FragCoord.xy/Resolution;
            Color = vec4(uv, 0.5, 1.0);
        }
        );
        #undef SHADER

        if (vdbIsFirstFrame())
            vdbLoadShader(0, fs);
        vdbClearColor(1,1,1,1);
        vdbBeginShader(0);
        vdbUniform2f("Resolution", (float)vdbGetFramebufferWidth(), (float)vdbGetFramebufferHeight());
        vdbEndShader();
    }
    VDBE();

    VDBB("3d");
    {
        static float t = 0.0f;
        t += 1.0f/60.0f;
        vdbMatrixEulerXYZ(0,0,0,0,0,0.1f*t);
        vdbOrtho(-2.0f*vdbGetAspectRatio(),+2.0f*vdbGetAspectRatio(),-2.0f,+2.0f);
        vdbTriangles();
        vdbVertex(-1, -1, 0);
        vdbVertex(+1, -1, 0);
        vdbVertex( 0, +1, 0);
        vdbEnd();

        if (vdbIsMouseOver(-1,-1)) { vdbNote(-1,-1,"-1,-1"); }
        if (vdbIsMouseOver(+1,-1)) { vdbNote(+1,-1,"+1,-1"); }
        if (vdbIsMouseOver( 0,+1)) { vdbNote( 0,+1,"0,+1"); }
    }
    VDBE();

    VDBB("mouse position NDC");
    {
        vdbClearColor(0.3f,0.5f,0.7f);
        vdbLines(2.0f);
        vdbColor(1.0f,1.0f,0.5f);
        vdbVertex(0.0f, 0.0f);
        vdbVertex(1.0f, 1.0f);
        vdbEnd();

        vdbOrtho(-1,+1,-1,+1);
        vdbVec2 m = vdbGetMousePosNDC();
        vdbNote(m.x,m.y,"MousePosNDC: %.2f %.2f", m.x, m.y);
    }
    VDBE();
    return 0;
}
