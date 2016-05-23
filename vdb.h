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

        // Cursor location in [-1, 1] coordinates relative to upper-left corner
        r32 X_NDC;
        r32 Y_NDC;

        struct button
        {
            bool Down;
            bool Released;
        } Left, Middle, Right;
    } Mouse;

    int WindowWidth;
    int WindowHeight;
    r32 DeltaTime;
    r32 ElapsedTime;
};

typedef std::function<void (vdb_input Input) > vdb_callback;
void vdb(char *Label, vdb_callback Callback);

#ifdef VDB_MY_CONFIG
#define VDBBS(Label) vdb(Label, [&](vdb_input Input) { using namespace ImGui;
#define VDBB(Label) vdb(Label, [&](vdb_input Input) { using namespace ImGui;
#define VDBE() });
#define VDB_SETTINGS_FILENAME "./.build/vdb.ini"
#define MOUSEX Input.Mouse.X_NDC
#define MOUSEY Input.Mouse.Y_NDC
#endif
