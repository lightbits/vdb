#pragma once
#include <functional>
#include "lib/imgui/imgui.h"

struct vdb_input
{
    struct mouse
    {
        // Cursor location in pixels relative to upper-left corner
        int X;
        int Y;

        // Cursor location in [-1, 1] coordinates relative to lower-left corner
        float X_NDC;
        float Y_NDC;

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
    float DeltaTime;
    float ElapsedTime;

    bool *TakeScreenshotNoDialog;
    char **ScreenshotFilename;
    bool *ScreenshotDrawGui;
    bool *ScreenshotDrawCursor;

    bool *StepOnce;
};

typedef std::function<void (vdb_input Input) > vdb_callback;
void vdb(const char *Label, vdb_callback Callback);

#define VDBBS(Label) vdbs(Label, [&](vdb_input Input) { using namespace ImGui;
#define VDBB(Label) vdb(Label, [&](vdb_input Input) { using namespace ImGui;
#define VDBE() });

#define vdbKeyDown(KEY) Input.Keys[SDL_SCANCODE_##KEY].Down
#define vdbKeyPressed(KEY) Input.Keys[SDL_SCANCODE_##KEY].Pressed
#define vdbKeyReleased(KEY) Input.Keys[SDL_SCANCODE_##KEY].Released
#define vdbMouseX Input.Mouse.X_NDC
#define vdbMouseY Input.Mouse.Y_NDC
#define vdbAspect (Input.WindowWidth / (float)Input.WindowHeight)

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
