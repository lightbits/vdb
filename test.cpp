// HOW TO COMPILE
// You will need SDL2 (https://wiki.libsdl.org/Installation):
// Linux: apt-get install libsdl2-dev
// OSX: brew install sdl2
// MSYS2: pacman -S mingw-w64-i686-SDL
// Windows: Download pre-compiled binaries and place SDL2.dll in executable directory.
//
// Then, run the build script for your platform to generate a library version of vdb.
// (build.bat for Windows, build.sh for Linux and Mac OSX).
//
// Windows
//   cl test.cpp -I"path\to\vdb\include" /MD /link /LIBPATH:"path\to\vdb\lib" /LIBPATH:"path\to\sdl2" vdb.lib SDL2.lib SDL2main.lib
//   Ensure that SDL2.dll is in the executable directory
//   You may need to also link against opengl32.lib user32.lib gdi32.lib shell32.lib
//
// On Linux and similar Unixes
//   g++ `sdl2-config --cflags` -Iinclude test.cpp -o test -Llib -lvdb `sdl2-config --libs` -lGL -ldl
//
// On Mac OSX
//   g++ `sdl2-config --cflags` -Iinclude test.cpp -o test -Llib -lvdb `sdl2-config --libs` -framework OpenGL -framework CoreFoundation
//
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
        vdbClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        ImGui::TextWrapped("Press F10 to continue.");
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

            ImGui::TextWrapped("Iteration: %d", i);
            ImGui::TextWrapped("F10 steps once.");
            ImGui::TextWrapped("F5 skips to the next VDBB/VDBE block.");
        }
        VDBE();
    }

    VDBB("Hello OpenGL");
    {
        vdbTriangles();
        vdbColor(1.0f, 0.5f, 0.5f, 1.0f); vdbVertex(-0.5f, -0.5f);
        vdbColor(0.5f, 1.0f, 0.5f, 1.0f); vdbVertex(+0.5f, -0.5f);
        vdbColor(0.5f, 0.5f, 1.0f, 1.0f); vdbVertex(+0.0f, +0.5f);
        vdbEnd();

        ImGui::TextWrapped("Inside a block you can draw things, like this triangle.");
    }
    VDBE();

    VDBB("3D");
    {
        static float t = 0.0f; t += 1.0f/60.0f;

        // You can also programmatically control the camera as well as
        // other aspects, such as depth testing, blend mode, etc.
        // See vdb.h for an API reference.
        #if 0
        vdbDepthTest(true);
        vdbDepthWrite(true);
        vdbClearDepth(1.0f);
        vdbCameraTurntable();
        // vdbCameraTrackball(); // try this one too!
        vdbRotateXYZ(-3.14f/2.0f,0,0);
        vdbPerspective(3.14f/4.0f, 0.1f, 10.0f);
        #endif

        vdbPointSize(6.0f);
        vdbBeginPoints();
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

        ImGui::TextWrapped("... or this 3D landscape! Click the 'Camera' tab and select a camera. Use mouse to move around.");
    }
    VDBE();

    VDBB("Hovering");
    {
        vdbDepthTest(true);
        vdbDepthWrite(true);
        vdbClearDepth(1.0f);
        vdbTranslate(0,0,-3);
        vdbRotateXYZ(-0.9f,0,0.7f + 0.3f*vdbGetMousePosNDC().x);
        vdbPerspective(3.14f/4.0f, 0.1f, 10.0f);
        vdbLineWidth(1.0f);
        vdbColor(0.4f,0.4f,0.4f,1);
        vdbLineGrid(-1,+1,-1,+1, 16);
        vdbPointSize3D(0.05f);
        vdbPointSegments(16);
        vdbBeginPoints();
        srand(128);
        for (int i = 0; i < 256; i++)
        {
            float x = -1.0f + 2.0f*(rand()%1000)/1000.0f;
            float y = -1.0f + 2.0f*(rand()%1000)/1000.0f;
            float z = -1.0f + 2.0f*(rand()%1000)/1000.0f;
            if (vdbWasMouseOver(x,y,z))
            {
                ImGui::SetTooltip("You can check if the mouse is over something,\nlike this dot, which has index %d", i);
            }
            vdbColor(0.5f+0.5f*x,0.5f+0.5f*y,0.5f+0.5f*z,1.0f);
            vdbVertex(x,y,z);
        }
        vdbEnd();

        vdbDepthTest(false);
        vdbPoints(16.0f);
        vdbColor(1.0f, 0.9f, 0.2f, 0.5f);
        float x, y, z;
        vdbGetMouseOverIndex(&x, &y, &z);
        vdbVertex(x, y, z);
        vdbEnd();
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
        vdbDrawImage(0, VDB_NEAREST, VDB_CLAMP);
        ImGui::TextWrapped("You can access variables outside the block, like this RGB image.");
    }
    VDBE();

    VDBB("ImGui");
    {
        vdbClearColor(0.99f, 0.55f, 0.38f, 1.0f);
        ImGui::ShowDemoWindow();
        ImGui::TextWrapped("VDB includes ImGui: https://github.com/ocornut/imgui/");
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

    VDBB("Notes");
    {
        static float t = 0.0f;
        t += 1.0f/60.0f;
        vdbRotateXYZ(0,0,0.1f*t);
        vdbOrtho(-2.0f*vdbGetAspectRatio(),+2.0f*vdbGetAspectRatio(),-2.0f,+2.0f);
        vdbTriangles();
        vdbVertex(-1, -1, 0);
        vdbVertex(+1, -1, 0);
        vdbVertex( 0, +1, 0);
        vdbEnd();

        if (vdbWasMouseOver(-1,-1)) { vdbNote(-1,-1,"-1,-1"); }
        if (vdbWasMouseOver(+1,-1)) { vdbNote(+1,-1,"+1,-1"); }
        if (vdbWasMouseOver( 0,+1)) { vdbNote( 0,+1,"0,+1"); }
    }
    VDBE();

    VDBB("Mouse");
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

    VDBB("RenderTexture");
    {
        static int tile = 0;
        int tiles_x = 16;
        int tiles_y = 16;
        int tile_w = 64;
        int tile_h = 64;
        int tile_x = tile % tiles_x;
        int tile_y = tile / tiles_x;

        vdbBeginRenderTexture(0, tiles_x*tile_w, tiles_y*tile_h, VDB_RGBA8U);
        vdbViewporti(tile_x*tile_w, tile_y*tile_h, tile_w, tile_h);
        if (tile == 0)
            vdbClearColor(1.0f, 0.73f, 0.22f, 1.0f);
        vdbTriangles();
        vdbColor(1.0f, 0.5f, 0.5f, 1.0f); vdbVertex(-0.5f, -0.5f);
        vdbColor(0.5f, 1.0f, 0.5f, 1.0f); vdbVertex(+0.5f, -0.5f);
        vdbColor(0.5f, 0.5f, 1.0f, 1.0f); vdbVertex(+0.0f, +0.5f);
        vdbEnd();
        vdbEndRenderTexture(0);
        vdbDrawRenderTexture(0);

        tile++;
        if (tile == tiles_x*tiles_y)
            tile = 0;
    }
    VDBE();

    VDBB("Test raycasting");
    {
        if (vdbIsFirstFrame())
        {
            #define glsl(x) "#version 150\n" #x
            const char *fs = glsl(
            uniform vec2 resolution;
            uniform vec2 ndc_offset;
            uniform mat4 model_to_view;
            uniform mat4 projection;
            out vec4 color;
            void main() {
                color = vec4(0.0,0.0,0.0,0.0);
                gl_FragDepth = 1.0;
                vec2 ndc = vec2(-1.0) + 2.0*gl_FragCoord.xy/resolution.xy;
                ndc += ndc_offset;
                mat4 view_to_model = inverse(model_to_view);
                vec3 rd = normalize((inverse(projection)*vec4(ndc, -1.0, 1.0)).xyz);
                rd = normalize((view_to_model*vec4(rd, 0.0)).xyz);
                vec3 ro = (view_to_model*vec4(0.0,0.0,0.0,1.0)).xyz;
                float t = 0.0;
                for (int i = 0; i < 64; i++) {
                    vec3 p = ro + rd*t;
                    float d = length(p) - 0.5;
                    if (d < 0.001) {
                        vec3 n = normalize(p);
                        color.rgb = 0.5*(vec3(0.5) + 0.5*n);
                        color.a = 1.0;
                        vec4 clip = projection*model_to_view*vec4(p, 1.0);
                        float z_ndc = clip.z/clip.w;
                        gl_FragDepth = 0.5 + 0.5*z_ndc;
                        break;
                    }
                    t += d;
                }
            });
            vdbLoadShader(0, fs);
        }

        float model_to_view[4*4];
        vdbGetMatrix(model_to_view);

        float projection[4*4];
        vdbGetProjection(projection);

        vdbBeginShader(0);
        vdbUniform2f("resolution", (float)vdbGetFramebufferWidth(), (float)vdbGetFramebufferHeight());
        vdbVec2 offset = vdbGetRenderOffset();
        vdbUniform2f("ndc_offset", offset.x, offset.y);
        vdbUniformMatrix4fv("model_to_view", model_to_view);
        vdbUniformMatrix4fv("projection", projection);
        vdbEndShader();

        vdbTriangles();
        vdbColor(1.0f, 0.5f, 0.5f, 1.0f); vdbVertex(-1.0f, 0.5f, -1.0f);
        vdbColor(0.5f, 1.0f, 0.5f, 1.0f); vdbVertex(+1.0f, 0.5f, -1.0f);
        vdbColor(0.5f, 0.5f, 1.0f, 1.0f); vdbVertex(+1.0f, 0.5f, +1.0f);
        vdbEnd();

        ImGui::TextWrapped("Mixing immediate mode rendering and ray-tracing. (Select a 3D camera.)");
    }
    VDBE();

    return 0;
}
