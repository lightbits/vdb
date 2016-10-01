// Example application (this compiles)
//
// #define SO_PLATFORM_IMPLEMENTATION
// #define SO_PLATFORM_MAIN
// #include "so_platform.h"
//
// void init(so_video_mode *mode)
// {
//     mode->title = "Hello sailor!";
//     mode->width = 640;
//     mode->height = 480;
//     mode->major = 3; // Request OpenGL 3.1 context
//     mode->minor = 1;
//     mode->samples = 4; // 4x multisampling
//     mode->x = -1; // Spawn anywhere
//     mode->y = -1;
//     mode->color_bits = 32;
//     mode->alpha_bits = 8;
//     mode->depth_bits = 24;
//     mode->stencil_bits = 0;
// }
//
// void tick(so_input input)
// {
//     glViewport(0, 0, input.width, input.height);
//     glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
//     glClear(GL_COLOR_BUFFER_BIT);
//
//     glBegin(GL_TRIANGLES);
//     glColor4f(1.0f, 0.2f, 0.1f, 1.0f);
//     glVertex2f(-0.5f, -0.5f);
//     glVertex2f(+0.5f, -0.5f);
//     glVertex2f(+0.0f, +0.5f);
//     glEnd();
// }
//

#ifndef SO_PLATFORM_INCLUDE
#define SO_PLATFORM_INCLUDE
#include <stdio.h>
#include <assert.h>

#ifdef _WIN32
#define VC_EXTRALEAN
#include <windows.h>
#include <gl/gl.h>
#endif

typedef struct
{
    int x, y; // Pixel coordinate from top-left
    float u, v; // [-1, +1] normalized coordinate from bottom-left
    int wheel;
} so_input_mouse;

typedef struct
{
    bool down; // Is down?
    bool released; // Was there a transition from down to up since last check?
    bool pressed; // Was there a transition from up to down since last check?
} so_input_button;

struct so_input
{
    so_input_mouse mouse;
    so_input_button left; // Left mouse button
    so_input_button middle; // Middle mouse button
    so_input_button right; // Right mouse button
    so_input_button *keys; // Array of key states accessible through SO_KEY_* enums
    int shift, ctrl, alt; // Key modifiers (1 if active)
    int width, height; // Window width and height in pixels
    float dt; // Seconds elapsed since previous so_loopWindow was called
    float t; // Seconds elapsed since so_openWindow was called

    char *text; // utf-8 null-terminated text input
};

void so_openWindow(const char *title,
                   int width,
                   int height,
                   int x = -1,
                   int y = -1,
                   int major = 3,
                   int minor = 1,
                   int multisamples = 4,
                   int color_bits = 32,
                   int alpha_bits = 8,
                   int depth_bits = 24,
                   int stencil_bits = 0);
bool so_loopWindow(so_input *input);
void so_closeWindow();
void so_setClipboardTextUTF8(const char *text);
const char *so_getClipboardTextUTF8();

#ifdef SO_PLATFORM_IMGUI
void so_imgui_init();
void so_imgui_shutdown();
void so_imgui_processEvents(so_input input);
#endif

#endif SO_PLATFORM_INCLUDE
#ifdef SO_PLATFORM_IMPLEMENTATION

static int so__charCountOfUTF8String(const char *text)
{
    int char_count = 0;
    const unsigned char *byte_index = (const unsigned char*)text;
    while (*byte_index)
    {
        unsigned char upper_two_bits = *byte_index & 0xC0;
        if (upper_two_bits != 0x80)
        {
            char_count++;
        }
        byte_index++;
    }
    return char_count;
}

#ifdef _WIN32
#define VC_EXTRALEAN
#include <windows.h>

// See <windowsx.h>
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#include <timeapi.h> // for timeBeginPeriod

enum
{
    SO_KEY_0 = 48,
    SO_KEY_1 = 49,
    SO_KEY_2 = 50,
    SO_KEY_3 = 51,
    SO_KEY_4 = 52,
    SO_KEY_5 = 53,
    SO_KEY_6 = 54,
    SO_KEY_7 = 55,
    SO_KEY_8 = 56,
    SO_KEY_9 = 57,

    SO_KEY_A = 65,
    SO_KEY_B = 66,
    SO_KEY_C = 67,
    SO_KEY_D = 68,
    SO_KEY_E = 69,
    SO_KEY_F = 70,
    SO_KEY_G = 71,
    SO_KEY_H = 72,
    SO_KEY_I = 73,
    SO_KEY_J = 74,
    SO_KEY_K = 75,
    SO_KEY_L = 76,
    SO_KEY_M = 77,
    SO_KEY_N = 78,
    SO_KEY_O = 79,
    SO_KEY_P = 80,
    SO_KEY_Q = 81,
    SO_KEY_R = 82,
    SO_KEY_S = 83,
    SO_KEY_T = 84,
    SO_KEY_U = 85,
    SO_KEY_V = 86,
    SO_KEY_W = 87,
    SO_KEY_X = 88,
    SO_KEY_Y = 89,
    SO_KEY_Z = 90,

    // See <winuser.h>
    SO_KEY_BACKSPACE   = 0x08,
    SO_KEY_TAB         = 0x09,
    SO_KEY_ENTER       = 0x0D,
    SO_KEY_SHIFT       = 0x10,
    SO_KEY_CTRL        = 0x11,
    SO_KEY_ALT         = 0x12,
    SO_KEY_PAUSE       = 0x13,
    SO_KEY_ESCAPE      = 0x1B,
    SO_KEY_SPACE       = 0x20,
    SO_KEY_PAGEUP      = 0x21,
    SO_KEY_PAGEDOWN    = 0x22,
    SO_KEY_END         = 0x23,
    SO_KEY_HOME        = 0x24,
    SO_KEY_LEFT        = 0x25,
    SO_KEY_UP          = 0x26,
    SO_KEY_RIGHT       = 0x27,
    SO_KEY_DOWN        = 0x28,
    SO_KEY_PRINTSCREEN = 0x2C,
    SO_KEY_INSERT      = 0x2D,
    SO_KEY_DELETE      = 0x2E,
    SO_KEY_LWIN        = 0x5B,
    SO_KEY_RWIN        = 0x5C,
    SO_KEY_NUMPAD0     = 0x60,
    SO_KEY_NUMPAD1     = 0x61,
    SO_KEY_NUMPAD2     = 0x62,
    SO_KEY_NUMPAD3     = 0x63,
    SO_KEY_NUMPAD4     = 0x64,
    SO_KEY_NUMPAD5     = 0x65,
    SO_KEY_NUMPAD6     = 0x66,
    SO_KEY_NUMPAD7     = 0x67,
    SO_KEY_NUMPAD8     = 0x68,
    SO_KEY_NUMPAD9     = 0x69,
    SO_KEY_MULTIPLY    = 0x6A,
    SO_KEY_ADD         = 0x6B,
    SO_KEY_SEPARATOR   = 0x6C,
    SO_KEY_SUBTRACT    = 0x6D,
    SO_KEY_DECIMAL     = 0x6E,
    SO_KEY_DIVIDE      = 0x6F,
    SO_KEY_F1          = 0x70,
    SO_KEY_F2          = 0x71,
    SO_KEY_F3          = 0x72,
    SO_KEY_F4          = 0x73,
    SO_KEY_F5          = 0x74,
    SO_KEY_F6          = 0x75,
    SO_KEY_F7          = 0x76,
    SO_KEY_F8          = 0x77,
    SO_KEY_F9          = 0x78,
    SO_KEY_F10         = 0x79,
    SO_KEY_F11         = 0x7A,
    SO_KEY_F12         = 0x7B,
    SO_KEY_F13         = 0x7C,
    SO_KEY_F14         = 0x7D,
    SO_KEY_F15         = 0x7E,
    SO_KEY_F16         = 0x7F,
    SO_KEY_F17         = 0x80,
    SO_KEY_F18         = 0x81,
    SO_KEY_F19         = 0x82,
    SO_KEY_F20         = 0x83,
    SO_KEY_F21         = 0x84,
    SO_KEY_F22         = 0x85,
    SO_KEY_F23         = 0x86,
    SO_KEY_F24         = 0x87,
    SO_KEY_LSHIFT      = 0xA0,
    SO_KEY_RSHIFT      = 0xA1,
    SO_KEY_LCTRL       = 0xA2,
    SO_KEY_RCTRL       = 0xA3,
    SO_KEY_LALT        = 0xA4,
    SO_KEY_RALT        = 0xA5,

    SO_KEY_COUNT       = 256
};

#include <gl/gl.h>
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "opengl32")
#pragma comment(lib, "winmm")

#define GL_SHADING_LANGUAGE_VERSION               0x8B8C
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// wglChoosePixelFormatARB
#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_NEED_PALETTE_ARB                    0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB             0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB              0x2006
#define WGL_SWAP_METHOD_ARB                     0x2007
#define WGL_NUMBER_OVERLAYS_ARB                 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                0x2009
#define WGL_TRANSPARENT_ARB                     0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB           0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB         0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB          0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB         0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB         0x203B
#define WGL_SHARE_DEPTH_ARB                     0x200C
#define WGL_SHARE_STENCIL_ARB                   0x200D
#define WGL_SHARE_ACCUM_ARB                     0x200E
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_RED_SHIFT_ARB                       0x2016
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_GREEN_SHIFT_ARB                     0x2018
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_BLUE_SHIFT_ARB                      0x201A
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_ALPHA_SHIFT_ARB                     0x201C
#define WGL_ACCUM_BITS_ARB                      0x201D
#define WGL_ACCUM_RED_BITS_ARB                  0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_AUX_BUFFERS_ARB                     0x2024
#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_SWAP_EXCHANGE_ARB                   0x2028
#define WGL_SWAP_COPY_ARB                       0x2029
#define WGL_SWAP_UNDEFINED_ARB                  0x202A
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C
#define WGL_SAMPLE_BUFFERS_ARB                  0x2041
#define WGL_SAMPLES_ARB                         0x2042

typedef HGLRC (WINAPI *wgl_create_context_attribs_arb)(HDC DC, HGLRC ShareContext, const int *Attribs);
typedef BOOL (WINAPI *wgl_choose_pixel_format_arb)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef BOOL (WINAPI *wgl_swap_interval_ext)(int Interval);

wgl_create_context_attribs_arb wglCreateContextAttribsARB;
wgl_choose_pixel_format_arb wglChoosePixelFormatARB;
wgl_swap_interval_ext wglSwapIntervalEXT;

HWND so_hwnd;
HGLRC so_hglrc;
bool so_ignore_destroy;
int so_wheel_delta;

struct so_platform_info
{
    const char *vendor;
    const char *renderer;
    const char *version;
    const char *glsl;
    const char *extensions;
} so_info;

static LRESULT CALLBACK so__windowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_DESTROY:
        {
            if (!so_ignore_destroy)
                PostQuitMessage(0);
            return 0;
        } break;

        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            if (delta > 0)
                so_wheel_delta = 1;
            if (delta < 0)
                so_wheel_delta = -1;
        } break;
    }
    return DefWindowProcW(wnd, msg, wparam, lparam);
}

// Returns a utf-8 encoded and null terminated string
const char *so_getClipboardTextUTF8()
{
    static char TextBuffer[1024];
    if (OpenClipboard(0))
    {
        if (IsClipboardFormatAvailable(CF_UNICODETEXT))
        {
            HGLOBAL Clipboard = GetClipboardData(CF_UNICODETEXT);
            if (Clipboard)
            {
                wchar_t *ClipboardText = (wchar_t*)GlobalLock(Clipboard);
                if (ClipboardText)
                {
                    // Hope that the ClipboardText is null terminated...
                    #if 1
                    WideCharToMultiByte(CP_UTF8, 0, ClipboardText, -1,
                                        TextBuffer, sizeof(TextBuffer), 0, 0);
                    #else
                    vdb_toUTF8(TextBuffer, ClipboardText, sizeof(TextBuffer));
                    #endif
                    GlobalUnlock(Clipboard);
                }
            }
        }
        CloseClipboard();
    }

    return TextBuffer;
}

// Text is expected to be encoded in utf8 and null terminated
void so_setClipboardTextUTF8(const char *Text)
{
    if (OpenClipboard(0))
    {
        EmptyClipboard();
        #if 1
        int BufferSizeInChars = MultiByteToWideChar(CP_UTF8, 0, Text, -1, NULL, 0);
        // The size includes the null-terminator
        int BufferSizeInBytes = BufferSizeInChars*sizeof(wchar_t);
        HGLOBAL Memory = GlobalAlloc(GMEM_MOVEABLE, BufferSizeInBytes);
        if (Memory)
        {
            wchar_t *Destination = (wchar_t*)GlobalLock(Memory);
            MultiByteToWideChar(CP_UTF8, 0, Text, -1,
                                Destination, BufferSizeInChars);
            GlobalUnlock(Memory);
            SetClipboardData(CF_UNICODETEXT, Memory);
        }
        #else
        int CharCount = so__charCountOfUTF8String(Text);
        // utf-16 has two bytes per character
        // We want to include the null-terminator, which becomes
        // two bytes in WCHAR format.
        int AllocLength = (CharCount+1)*2;
        HGLOBAL Memory = GlobalAlloc(GMEM_MOVEABLE, AllocLength);
        if (Memory)
        {
            wchar_t *Destination = (wchar_t*)GlobalLock(Memory);
            vdb_fromUTF8(Destination, Text, AllocLength);
            GlobalUnlock(Memory);
            SetClipboardData(CF_UNICODETEXT, Memory);
        }
        #endif
        CloseClipboard();
    }
}

// #ifdef SO_PLATFORM_ALLOC_CONSOLE
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
// #endif

wchar_t *so__from_utf8_temp(const char *text)
{
    static wchar_t buf[1024];
    int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    assert(len < sizeof(buf) / sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, text, -1, buf, len);
    return buf;
}

HWND so__createWindow(HINSTANCE instance, WNDCLASSW wndclass, const char *title, int width, int height, int x, int y)
{
    DWORD flags = WS_OVERLAPPEDWINDOW;
    HWND wnd = CreateWindowExW(
        0,
        wndclass.lpszClassName,
        so__from_utf8_temp(title),
        flags,
        x == -1 ? CW_USEDEFAULT : x,
        y == -1 ? CW_USEDEFAULT : y,
        width,
        height,
        0,
        0,
        instance,
        0);

    if (!wnd)
    {
        // Failed to open window
        assert(false);
    }

    return wnd;
}

void so_openWindow(
    const char *title,
    int width,
    int height,
    int x,
    int y,
    int major,
    int minor,
    int multisamples,
    int color_bits,
    int alpha_bits,
    int depth_bits,
    int stencil_bits)
{
    so_ignore_destroy = false;
    so_wheel_delta = 0;

    HINSTANCE instance = GetModuleHandle(0);

    WNDCLASSW wndclass = {};
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.hInstance = instance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszMenuName = L"so_platform_class";
    wndclass.lpszClassName = L"so_platform_class";
    wndclass.lpfnWndProc = so__windowProc;

    if (!RegisterClassW(&wndclass))
    {
        assert(false);
    }

    HWND wnd = so__createWindow(instance, wndclass, title, width, height, x, y);

    // Create a legacy OpenGL context
    HGLRC rc = 0;
    {
        HDC dc = GetDC(wnd);

        PIXELFORMATDESCRIPTOR want_fmt = {0};
        want_fmt.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        want_fmt.nVersion = 1;
        want_fmt.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        want_fmt.iPixelType = PFD_TYPE_RGBA;
        want_fmt.cColorBits = color_bits;
        want_fmt.cAlphaBits = alpha_bits;
        want_fmt.cAccumBits = 0;
        want_fmt.cDepthBits = depth_bits;
        want_fmt.cStencilBits = stencil_bits;
        want_fmt.iLayerType = PFD_MAIN_PLANE;

        int got_index = ChoosePixelFormat(dc, &want_fmt);
        if (!got_index)
        {
            assert(false);
        }

        PIXELFORMATDESCRIPTOR got_fmt;
        if (!DescribePixelFormat(dc, got_index, sizeof(got_fmt), &got_fmt))
        {
            assert(false);
        }

        if (!SetPixelFormat(dc, got_index, &got_fmt))
        {
            assert(false);
        }

        HGLRC rc = wglCreateContext(dc);
        if (!rc)
        {
            assert(false);
        }

        if (!wglMakeCurrent(dc, rc))
        {
            assert(false);
        }

        so_info.renderer = (const char*)glGetString(GL_RENDERER);
        so_info.vendor = (const char*)glGetString(GL_VENDOR);
        so_info.version = (const char*)glGetString(GL_VERSION);
        so_info.extensions = (const char*)glGetString(GL_EXTENSIONS);
        so_info.glsl = 0;

        ReleaseDC(wnd, dc);
    }

    bool need_modern_context = false;
    if (major > 1 || multisamples > 0)
        need_modern_context = true;

    if (need_modern_context)
    {
        wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb)wglGetProcAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (wgl_create_context_attribs_arb)wglGetProcAddress("wglCreateContextAttribsARB");
        assert(wglChoosePixelFormatARB);
        assert(wglCreateContextAttribsARB);

        // SetPixelFormat can only be called once per window
        // So we destroy the current window and recreate it.
        wglDeleteContext(rc);
        so_ignore_destroy = true;
        DestroyWindow(wnd);
        so_ignore_destroy = false;
        wnd = so__createWindow(instance, wndclass, title, width, height, x, y);
        HDC dc = GetDC(wnd);

        int pfd_attribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB,     color_bits,
            WGL_DEPTH_BITS_ARB,     depth_bits,
            WGL_STENCIL_BITS_ARB,   stencil_bits,
            WGL_SAMPLE_BUFFERS_ARB, multisamples > 0 ? 1 : 0,
            WGL_SAMPLES_ARB,        multisamples,
            0
        };

        int got_index = 0;
        UINT num_matches = 0;
        if (!wglChoosePixelFormatARB(dc, pfd_attribs, 0, 1, &got_index, &num_matches))
        {
            assert(false);
        }

        PIXELFORMATDESCRIPTOR got_fmt;
        if (!DescribePixelFormat(dc, got_index, sizeof(got_fmt), &got_fmt))
        {
            assert(false);
        }

        if (!SetPixelFormat(dc, got_index, &got_fmt))
        {
            assert(false);
        }

        int ctx_attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, major,
            WGL_CONTEXT_MINOR_VERSION_ARB, minor,
            WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_DEBUG_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        rc = wglCreateContextAttribsARB(dc, 0, ctx_attribs);
        if (!rc)
        {
            assert(false);
        }

        if (!wglMakeCurrent(dc, rc))
        {
            assert(false);
        }

        so_info.glsl = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

        ReleaseDC(wnd, dc);
    }

    // TODO: so_printOpenGLInfo
    // printf("Extensions: %s\n",  so_info.extensions);
    // printf("Vendor: %s\n",      so_info.vendor);
    // printf("Renderer: %s\n",    so_info.renderer);
    // printf("Version: %s\n",     so_info.version);
    // printf("GLSLVersion: %s\n", so_info.glsl);

    wglSwapIntervalEXT = (wgl_swap_interval_ext)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(1);
    }

    ShowWindow(wnd, SW_SHOW);

    // Increase timer resolution. I think Windows automatically resets the
    // period when the process closes.
    timeBeginPeriod(1);

    so_hglrc = rc;
    so_hwnd = wnd;
}

bool so_loopWindow(so_input *input)
{
    static so_input_button keys[SO_KEY_COUNT];
    input->keys = keys;

    for (int i = 0; i < SO_KEY_COUNT; i++)
    {
        keys[i].pressed = false;
        keys[i].released = false;
    }

    input->left.released = false;
    input->left.pressed = false;
    input->middle.released = false;
    input->middle.pressed = false;
    input->right.released = false;
    input->right.pressed = false;

    static char text_utf16[1024];
    int cur_text_utf16 = 0;

    MSG msg;
    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
            case WM_QUIT:
            {
                return false;
            } break;

            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                if (msg.wParam < SO_KEY_COUNT)
                {
                    if (!keys[msg.wParam].down)
                        keys[msg.wParam].pressed = true;
                    keys[msg.wParam].down = true;
                }

                if (msg.wParam == VK_MENU) input->alt = true;
                if (msg.wParam == VK_CONTROL) input->ctrl = true;
                if (msg.wParam == VK_SHIFT) input->shift = true;
                TranslateMessage(&msg);
            } break;

            case WM_SYSKEYUP:
            case WM_KEYUP:
            {
                if (msg.wParam < SO_KEY_COUNT)
                {
                    if (keys[msg.wParam].down)
                        keys[msg.wParam].released = true;
                    keys[msg.wParam].down = false;
                }

                if (msg.wParam == VK_MENU) input->alt = false;
                if (msg.wParam == VK_CONTROL) input->ctrl = false;
                if (msg.wParam == VK_SHIFT) input->shift = false;
            } break;

            case WM_CHAR:
            {
                if (cur_text_utf16 < sizeof(text_utf16)-1)
                {
                    text_utf16[cur_text_utf16] = (char)msg.wParam;
                    text_utf16[cur_text_utf16+1] = 0;
                    cur_text_utf16++;
                }
            } break;

            #define so__btn_up(btn) { if ((btn).down) btn.released = true; btn.down = false; }
            #define so__btn_down(btn) { if (!(btn).down) btn.pressed = true; btn.down = true; }

            case WM_LBUTTONDOWN: so__btn_down(input->left); break;
            case WM_RBUTTONDOWN: so__btn_down(input->right); break;
            case WM_MBUTTONDOWN: so__btn_down(input->middle); break;
            case WM_LBUTTONUP: so__btn_up(input->left); break;
            case WM_RBUTTONUP: so__btn_up(input->right); break;
            case WM_MBUTTONUP: so__btn_up(input->middle); break;

            case WM_MOUSEMOVE:
            {
                int x = GET_X_LPARAM(msg.lParam);
                int y = GET_Y_LPARAM(msg.lParam);
                input->mouse.x = x;
                input->mouse.y = y;
            } break;

            default:
            {
                DispatchMessageW(&msg);
            }
        }
    }

    input->mouse.wheel = so_wheel_delta;
    so_wheel_delta = 0;

    // Convert per-frame input of utf16 characters to utf8 stream
    if (cur_text_utf16 > 0)
    {
        static char text_utf8[1024];
        if (WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)text_utf16, -1, text_utf8, sizeof(text_utf8), 0, 0) != 0)
        {
            input->text = text_utf8;
            cur_text_utf16 = 0;
        }
    }
    else
    {
        input->text = 0;
    }

    // Fetch latest width and height of window
    RECT rect;
    GetClientRect(so_hwnd, &rect);
    input->width = rect.right-rect.left;
    input->height = rect.bottom-rect.top;

    // Translate mouse pixel coordinates to normalized coordinates
    input->mouse.u = -1.0f + 2.0f * (input->mouse.x / (float)input->width);
    input->mouse.v = -1.0f + 2.0f * ((input->height-input->mouse.y-1) / (float)input->height);

    static LONGLONG prevcount = 0;
    static LONGLONG initcount = 0;
    if (!prevcount)
    {
        LARGE_INTEGER perfcount;
        QueryPerformanceCounter(&perfcount);
        prevcount = perfcount.QuadPart;
        initcount = perfcount.QuadPart;
    }

    {
        LARGE_INTEGER perfcount;
        LARGE_INTEGER perffreq;
        QueryPerformanceCounter(&perfcount);
        QueryPerformanceFrequency(&perffreq);

        LONGLONG count = perfcount.QuadPart;
        LONGLONG freq = perffreq.QuadPart;
        input->dt = (float)(count - prevcount) / (float)freq;
        input->t = (float)(count - initcount) / (float)freq;

        prevcount = count;
    }

    return true;
}

void so_swapBuffers()
{
    HDC dc = GetDC(so_hwnd);
    SwapBuffers(dc);
    ReleaseDC(so_hwnd, dc);
}

void so_sleep(int milliseconds)
{
    Sleep(milliseconds);
}

void so_closeWindow()
{
    wglDeleteContext(so_hglrc);
    DestroyWindow(so_hwnd);
}

#endif

#ifdef SO_PLATFORM_IMGUI
GLuint so_imgui_font_texture;

void so__imgui_drawLists(ImDrawData* draw_data)
{
    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

    // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render command lists
    #define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
        const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
    #undef OFFSETOF

    // Restore modified state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}

void so_imgui_init()
{
    ImGuiIO& io = ImGui::GetIO();

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_Tab] = SO_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SO_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SO_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SO_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SO_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SO_KEY_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SO_KEY_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SO_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = SO_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = SO_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SO_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SO_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = SO_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SO_KEY_A;
    io.KeyMap[ImGuiKey_C] = SO_KEY_C;
    io.KeyMap[ImGuiKey_V] = SO_KEY_V;
    io.KeyMap[ImGuiKey_X] = SO_KEY_X;
    io.KeyMap[ImGuiKey_Y] = SO_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = SO_KEY_Z;

    io.RenderDrawListsFn = so__imgui_drawLists;
    io.SetClipboardTextFn = so_setClipboardTextUTF8;
    io.GetClipboardTextFn = so_getClipboardTextUTF8;

    ImGuiStyle &style = ImGui::GetStyle();
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    #ifdef SO_PLATFORM_IMGUI_FONT
    io.Fonts->AddFontFromFileTTF(SO_PLATFORM_IMGUI_FONT);
    // io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/SourceSansPro-SemiBold.ttf", 18.0f);
    #endif
    // io.IniFilename = ...

    // Build texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &so_imgui_font_texture);
    glBindTexture(GL_TEXTURE_2D, so_imgui_font_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)so_imgui_font_texture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);
}

void so_imgui_shutdown()
{
    glDeleteTextures(1, &so_imgui_font_texture);
    ImGui::GetIO().Fonts->TexID = 0;
    so_imgui_font_texture = 0;
    ImGui::Shutdown();
}

void so_imgui_processEvents(so_input input)
{
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = input.dt > 0.0f ? input.dt : 1.0f / 60.0f;
    io.DisplaySize.x = input.width;
    io.DisplaySize.y = input.height;

    io.MousePos.x = input.mouse.x;
    io.MousePos.y = input.mouse.y;

    io.MouseWheel = input.mouse.wheel;

    io.MouseDown[0] = input.left.pressed || input.left.down;
    io.MouseDown[1] = input.right.pressed || input.right.down;
    io.MouseDown[2] = input.middle.pressed || input.middle.down;

    if (input.text)
        io.AddInputCharactersUTF8(input.text);

    for (int key = 0; key < SO_KEY_COUNT; key++)
        io.KeysDown[key] = input.keys[key].down;

    io.KeyShift = input.shift;
    io.KeyCtrl = input.ctrl;
    io.KeyAlt = input.alt;
}
#endif

#ifdef SO_PLATFORM_MAIN
struct so_video_mode
{
    const char *title;
    int width;
    int height;
    int major;
    int minor;
    int samples;
    int x;
    int y;
    int color_bits;
    int alpha_bits;
    int depth_bits;
    int stencil_bits;
};
void init(so_video_mode *mode);
void tick(so_input input);
int main(int argc, char **argv)
{
    so_video_mode mode;
    mode.title = "Hello sailor!";
    mode.width = 640;
    mode.height = 480;
    mode.major = 3;
    mode.minor = 1;
    mode.samples = 4;
    mode.x = -1;
    mode.y = -1;
    mode.color_bits = 32;
    mode.alpha_bits = 8;
    mode.depth_bits = 24;
    mode.stencil_bits = 0;
    init(&mode);

    so_openWindow(mode.title,
                  mode.width,
                  mode.height,
                  mode.x,
                  mode.y,
                  mode.major,
                  mode.minor,
                  mode.samples,
                  mode.color_bits,
                  mode.alpha_bits,
                  mode.depth_bits,
                  mode.stencil_bits);

    so_input input = {0};
    while (so_loopWindow(&input))
    {
        glViewport(0, 0, input.width, input.height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        tick(input);

        so_swapBuffers();
    }
    so_closeWindow();
    return 0;
}
#endif

#endif // SO_PLATFORM_IMPLEMENTATION
