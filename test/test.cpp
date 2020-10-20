/*
*************************************
How to compile
*************************************

1. Get the SDL2 binaries (https://wiki.libsdl.org/Installation):

      Linux: apt-get install libsdl2-dev
    Mac OSX: brew install sdl2
      MSYS2: pacman -S mingw-w64-i686-SDL
    Windows: Download pre-compiled binaries and place SDL2.dll in this directory.

2. Run build_static_lib (.sh or .bat) for your platform to compile vdb as a library.

3. Run make or build.bat or compile from commandline

*************************************
Compiling from commandline
*************************************

Linux and similar Unixes
  g++ `sdl2-config --cflags` -I../include test.cpp -o test -L../lib -lvdb `sdl2-config --libs` -lGL -ldl
Mac OSX
  g++ `sdl2-config --cflags` -I../include test.cpp -o test -L../lib -lvdb `sdl2-config --libs` -framework OpenGL -framework CoreFoundation
Windows
  cl test.cpp -I"path\to\vdb\include" /MD /link /LIBPATH:"path\to\vdb\lib" /LIBPATH:"path\to\sdl2" vdb.lib SDL2.lib SDL2main.lib
  Ensure that SDL2.dll is in the executable directory
  You may need to also link against opengl32.lib user32.lib gdi32.lib shell32.lib
*
*/
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
        vdbNoteAlign(0.5f, 0.5f);
        vdbNote(0.0f, 0.0f, "Welcome to vdb!\nPress F10 to continue.");
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

            vdbNote(0.0f, 0.0f, "F10 steps once.\nF5 skips to the next VDBB/VDBE block.\nIteration: %d\n", i);
        }
        VDBE();
    }

    VDBB("Hello OpenGL");
    {
        vdbClearColor(0.22f,0.22f,0.22f,0.0f);
        vdbBeginTriangles();
        vdbColor(1.0f, 0.5f, 0.5f, 1.0f); vdbVertex(-0.5f, -0.5f);
        vdbColor(0.5f, 1.0f, 0.5f, 1.0f); vdbVertex(+0.5f, -0.5f);
        vdbColor(0.5f, 0.5f, 1.0f, 1.0f); vdbVertex(+0.0f, +0.5f);
        vdbEnd();

        vdbNote(0.0f, 0.5f, "Inside a block you can draw things.\nLike this triangle.");
    }
    VDBE();

    // Hints can be used to initialize VDB settings upon entering a block.
    // Settings can still be changed within the block, but changes will not
    // persist across restarts of the program.
    vdbHint(VDB_VIEW_SCALE, 2.0f);
    vdbHint(VDB_ORIENTATION, VDB_Z_UP);
    vdbHint(VDB_CAMERA_TYPE, VDB_TURNTABLE);
    VDBB("3D");
    {
        static float t = 0.0f; t += 1.0f/60.0f;
        int nx = 64;
        int ny = 64;
        vdbPointSize(6.0f);
        vdbBeginPoints();
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
        vdbEnd();

        vdbNote(0.0f, 0.5f, "...or this 3D landscape.\n(Click the 'View' tab and select a camera.\nUse mouse to move around.)");
    }
    VDBE();

    VDBB("Shaders");
    {
        if (vdbIsFirstFrame())
        {
            vdbLoadShader(0, // <- This number is the "slot", which is used again below to access the shader
            "uniform float iTime;\n"
            "void mainImage(out vec4 fragColor, in vec2 fragCoord)\n"
            "{\n"
            "    vec2 uv = fragCoord.xy/iResolution.xy;\n"
            "    fragColor = vec4(uv, 0.5 + 0.5*sin(iTime), 1.0);\n"
            "}\n");
        }

        static float iTime = 0.0f;
        iTime += 1.0f/60.0f;

        vdbBeginShader(0);
        vdbUniform1f("iTime", iTime); // <- Shader uniforms must be specified inside the vdb(Begin|End)Shader block
        vdbEndShader();

        vdbNote(0.0f, 0.5f, "...or custom fragment shaders!");
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
        vdbPointSize(16.0f);
        vdbBeginPoints();
        vdbColor(1.0f, 0.9f, 0.2f, 0.5f);
        float x, y, z;
        vdbGetMouseOverIndex(&x, &y, &z);
        vdbVertex(x, y, z);
        vdbEnd();
    }
    VDBE();

    const int width = 128;
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
        if (vdbIsFirstFrame())
            vdbLoadImageUint8(0, data, width, height, 3);
        vdbDrawImage(0, -1.0f,-1.0f,2.0f,2.0f, VDB_NEAREST, VDB_CLAMP);
        vdbNote(0.0f, 0.0f, "You can access variables outside the block,\nlike this RGB image.");
    }
    VDBE();

    VDBB("Dear ImGui");
    {
        vdbClearColor(0.99f, 0.55f, 0.38f, 1.0f);
        ImGui::ShowDemoWindow();
        ImGui::TextWrapped("vdb includes Dear ImGui: https://github.com/ocornut/imgui/");
    }
    VDBE();

    VDBB("Notes");
    {
        static float t = 0.0f;
        t += 1.0f/60.0f;
        vdbRotateXYZ(0,0,0.1f*t);
        vdbOrtho(-2.0f*vdbGetAspectRatio(),+2.0f*vdbGetAspectRatio(),-2.0f,+2.0f);
        vdbBeginTriangles();
        vdbColor(1.0f, 1.0f, 1.0f, 1.0f);
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
        vdbClearColor(0.3f,0.5f,0.7f,1.0f);
        vdbLineWidth(2.0f);
        vdbBeginLines();
        vdbColor(1.0f,1.0f,0.5f);
        vdbVertex(0.0f, 0.0f);
        vdbVertex(1.0f, 1.0f);
        vdbEnd();

        vdbOrtho(-1,+1,-1,+1);
        vdbVec2 m = vdbGetMousePosNDC();
        vdbNote(m.x,m.y,"MousePosNDC: %.2f %.2f", m.x, m.y);
    }
    VDBE();

    VDBB("Offscreen rendering");
    {
        static int tile = 0;
        int tiles_x = 16;
        int tiles_y = 16;
        int tile_w = 64;
        int tile_h = 64;
        int tile_x = tile % tiles_x;
        int tile_y = tile / tiles_x;

        vdbBeginRenderTarget(0, {.width=tiles_x*tile_w, .height=tiles_y*tile_h, .format=VDB_RGBA8});
        vdbViewporti(tile_x*tile_w, tile_y*tile_h, tile_w, tile_h);
        if (tile == 0)
            vdbClearColor(1.0f, 0.73f, 0.22f, 1.0f);
        vdbBeginTriangles();
        vdbColor(1.0f, 0.5f, 0.5f, 1.0f); vdbVertex(-0.5f, -0.5f);
        vdbColor(0.5f, 1.0f, 0.5f, 1.0f); vdbVertex(+0.5f, -0.5f);
        vdbColor(0.5f, 0.5f, 1.0f, 1.0f); vdbVertex(+0.0f, +0.5f);
        vdbEnd();
        vdbEndRenderTarget();
        vdbDrawRenderTarget(0);

        tile++;
        if (tile == tiles_x*tiles_y)
            tile = 0;
    }
    VDBE();

    VDBB("Shaders");
    {
        if (vdbIsFirstFrame())
        {
            #define glsl(x) #x
            const char *fs = glsl(
            float scene(vec3 p) {
                return length(p) - 0.5;
            }
            void mainImage(out vec4 fragColor, in vec2 fragCoord) {
                vec2 ndc = vec2(-1.0) + 2.0*fragCoord.xy/iResolution.xy;

                // Compute the ray origin and direction for this pixel
                mat4 inv_pvm = inverse(iPVM);
                vec4 ro = inv_pvm*vec4(ndc, +0.0, 1.0);
                ro.xyz /= ro.w;
                vec4 rd = inv_pvm*vec4(ndc, +1.0, 1.0);
                rd.xyz /= rd.w;
                rd = normalize(rd - ro);

                // Color and depth if we don't hit anything
                fragColor = vec4(0.0,0.0,0.0,0.0);
                gl_FragDepth = 1.0;

                // Simple ray-marched scene
                float t = 0.0;
                for (int i = 0; i < 64; i++) {
                    vec3 p = ro.xyz + rd.xyz*t;
                    float d = scene(p);
                    if (d < 0.001) { // We hit the scene.

                        // Color based on normal
                        fragColor.rgb = 0.5*(vec3(0.5) + 0.5*normalize(p));
                        fragColor.a = 1.0;

                        // Overwrite fragment depth (see OpenGL Spec: Coordinate Transformations).
                        // Note that gl_DepthRange.near|far is not the same as the near and far
                        // values used to form the projection matrix. vdb sets gl_DepthRange to
                        // [0, 1] regardless, so their involvement here can be omitted.
                        vec4 clip = iPVM*vec4(p, 1.0);
                        float z_d = clip.z/clip.w;
                        gl_FragDepth = gl_DepthRange.near + (gl_DepthRange.diff)*(0.5 + 0.5*z_d);
                        break;
                    }
                    t += d;
                }
            });
            vdbLoadShader(0, fs);
        }

        vdbBeginShader(0);
        vdbEndShader();

        // demonstrate that we can mix immediate mode rendering with ray tracing
        vdbBeginTriangles();
        vdbColor(1.0f, 0.5f, 0.5f, 1.0f); vdbVertex(-1.0f, 0.5f, -1.0f);
        vdbColor(0.5f, 1.0f, 0.5f, 1.0f); vdbVertex(+1.0f, 0.5f, -1.0f);
        vdbColor(0.5f, 0.5f, 1.0f, 1.0f); vdbVertex(+1.0f, 0.5f, +1.0f);
        vdbEnd();

        ImGui::TextWrapped("Mixing immediate mode rendering and ray-tracing. (Select a 3D camera.)");
    }
    VDBE();

    vdbHint(VDB_CAMERA_TYPE, VDB_PLANAR);
    VDBB("Colormaps");
    {
        // VDB supports most of the colormaps provided by Matplotlib,
        // accessed using a string which follows Matplotlib's naming:
        // https://matplotlib.org/tutorials/colors/colormaps.html
        ImGui::Text("VDB supports Matplotlib colormaps.");
        int n = vdbSetColormap("tab10");
        vdbPointSegments(4);
        vdbPointSize3D(8.0f/n);
        vdbBeginPoints();
        for (int i = 0; i < n; i++)
        {
            float x = -4.0f + 8.0f*i/n + 0.5f*8.0f/n;
            vdbNextColor();
            vdbVertex(x, 0.0f);
        }
        vdbEnd();

        vdbTranslate(0.0f,2.0f,0.0f);
        vdbSetColormap("viridis");
        vdbBeginTriangles();
        for (int i = 0; i < 256; i++)
        {
            float t1 = i/256.0f;
            float t2 = (i+1)/256.0f;
            float x1 = -4.0f + 8.0f*t1;
            float x2 = -4.0f + 8.0f*t2;
            vdbColor(t1); vdbVertex(x1, -0.5f);
            vdbColor(t2); vdbVertex(x2, -0.5f);
            vdbColor(t2); vdbVertex(x2, +0.5f);
            vdbColor(t2); vdbVertex(x2, +0.5f);
            vdbColor(t1); vdbVertex(x1, +0.5f);
            vdbColor(t1); vdbVertex(x1, -0.5f);
        }
        vdbEnd();

        vdbTranslate(0.0f,2.0f,0.0f);
        vdbSetColormap("twilight");
        vdbBeginTriangles();
        const float two_pi = 2.0f*3.1415926535897932384626433832795f;
        for (int i = 0; i < 256; i++)
        {
            float t1 = i/256.0f;
            float t2 = (i+1)/256.0f;
            vdbColor(t1); vdbVertex(0.5f*cosf(two_pi*t1), 0.5f*sinf(two_pi*t1));
            vdbColor(t1); vdbVertex(1.0f*cosf(two_pi*t1), 1.0f*sinf(two_pi*t1));
            vdbColor(t2); vdbVertex(1.0f*cosf(two_pi*t2), 1.0f*sinf(two_pi*t2));
            vdbColor(t2); vdbVertex(1.0f*cosf(two_pi*t2), 1.0f*sinf(two_pi*t2));
            vdbColor(t2); vdbVertex(0.5f*cosf(two_pi*t2), 0.5f*sinf(two_pi*t2));
            vdbColor(t1); vdbVertex(0.5f*cosf(two_pi*t1), 0.5f*sinf(two_pi*t1));
        }
        vdbEnd();
    }
    VDBE();

    return 0;
}
