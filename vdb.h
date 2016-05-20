#pragma once
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <gl/gl.h>
#elif __linux__
#error "Include my stuff"
#endif
#include "lib/imgui.h"
#include <functional>
#include <stdint.h>
typedef float    vdb_r32;
typedef uint64_t vdb_u64;
typedef uint32_t vdb_u32;
typedef uint16_t vdb_u16;
typedef uint8_t  vdb_u08;
typedef int64_t  vdb_s64;
typedef int32_t  vdb_s32;
typedef int16_t  vdb_s16;
typedef int8_t   vdb_s08;

struct vdb_input
{
    struct mouse
    {
        // Position in pixels relative upper-left corner
        float X;
        float Y;

        struct button
        {
            bool WasDownThisFrame;
            bool IsDown;
        } Left, Middle, Right;
    } Mouse;

    bool Keys[256];

    int WindowWidth;
    int WindowHeight;
    float DeltaTime;
    float ElapsedTime;
};

typedef std::function<void (vdb_input Input) > vdb_callback;
void vdb(vdb_callback Callback);

#define IFKEY(Key) if (Input.Keys[Key])
#define MOUSEX Input.Mouse.X
#define MOUSEY Input.Mouse.Y

#ifdef VDB_MY_CONFIG
typedef float    r32;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u08;
typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s08;
#endif
