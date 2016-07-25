#pragma once
#include <functional>
#include <stdint.h>
#include "lib/imgui/imgui.h"
typedef float    r32;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u08;
typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s08;

struct vdb_input
{
    struct mouse
    {
        // Cursor location in pixels relative to upper-left corner
        int X;
        int Y;

        // Cursor location in [-1, 1] coordinates relative to lower-left corner
        r32 X_NDC;
        r32 Y_NDC;

        struct button
        {
            bool Down;
            bool Released;
        } Left, Middle, Right;
    } Mouse;

    struct key
    {
        bool Down;     //  IsDown
        bool Pressed;  // !WasDown &&  IsDown
        bool Released; //  WasDown && !IsDown
    } *Keys; // May be accessed by an SDL_SCANCODE_*** value

    int WindowWidth;
    int WindowHeight;
    r32 DeltaTime;
    r32 ElapsedTime;

    bool *TakeScreenshotNoDialog;
    char **ScreenshotFilename;
    bool *ScreenshotDrawGui;
    bool *ScreenshotDrawCursor;
};

typedef std::function<void (vdb_input Input) > vdb_callback;
void vdb(const char *Label, vdb_callback Callback);

#define VDBBS(Label) vdbs(Label, [&](vdb_input Input) { using namespace ImGui;
#define VDBB(Label) vdb(Label, [&](vdb_input Input) { using namespace ImGui;
#define VDBE() });

#define vdbKeyDown(KEY) Input.Keys[SDL_SCANCODE_##KEY].Down
#define vdbKeyPressed(KEY) Input.Keys[SDL_SCANCODE_##KEY].Pressed
#define vdbKeyReleased(KEY) Input.Keys[SDL_SCANCODE_##KEY].Released
#define vdbMouseX() Input.Mouse.X_NDC
#define vdbMouseY() Input.Mouse.Y_NDC
#define vdbAspect() ((r32)Input.WindowWidth / (r32)Input.WindowHeight)

#ifdef VDB_MY_CONFIG
#ifndef VDB_SETTINGS_FILENAME
#define VDB_SETTINGS_FILENAME "./.build/vdb.ini"
#endif
#ifndef VDB_IMGUI_INI_FILENAME
#define VDB_IMGUI_INI_FILENAME "./.build/imgui.ini"
#endif
#define MOUSEX Input.Mouse.X_NDC
#define MOUSEY Input.Mouse.Y_NDC
#endif

#ifndef VDB_IMGUI_INI_FILENAME
#define VDB_IMGUI_INI_FILENAME "./imgui.ini"
#endif

#ifndef VDB_SETTINGS_FILENAME
#define VDB_SETTINGS_FILENAME "./vdb.ini"
#endif

#ifndef VDB_GL_MAJOR
#define VDB_GL_MAJOR 1
#endif

#ifndef VDB_GL_MINOR
#define VDB_GL_MINOR 5
#endif

#ifndef VDB_MULTISAMPLES
#define VDB_MULTISAMPLES 4
#endif
