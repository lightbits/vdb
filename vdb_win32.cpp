// @ ifdef msvc
// @ subsystem: windows and entrypoint: crt?
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "opengl32")
#pragma comment(lib, "winmm")
#include <windowsx.h> // GET_X_LPARAM, GET_Y_LPARAM
#include <mmsystem.h> // timeBeginPeriod

// https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
enum vdb_keys
{
    VDB_KEY_TAB = VK_TAB,
    VDB_KEY_LEFT = VK_LEFT,
    VDB_KEY_RIGHT = VK_RIGHT,
    VDB_KEY_UP = VK_UP,
    VDB_KEY_DOWN = VK_DOWN,
    VDB_KEY_PAGEUP = VK_PRIOR,
    VDB_KEY_PAGEDOWN = VK_NEXT,
    VDB_KEY_HOME = VK_HOME,
    VDB_KEY_END = VK_END,
    VDB_KEY_DELETE = VK_DELETE,
    VDB_KEY_BACK = VK_BACK,
    VDB_KEY_ENTER = VK_RETURN,
    VDB_KEY_ESCAPE = VK_ESCAPE,
    VDB_KEY_A = 0x41,
    VDB_KEY_C = 0x43,
    VDB_KEY_V = 0x56,
    VDB_KEY_X = 0x58,
    VDB_KEY_Y = 0x59,
    VDB_KEY_Z = 0x5A
};

typedef BOOL (WINAPI *wgl_swap_interval_ext)(int Interval);
wgl_swap_interval_ext wglSwapIntervalEXT;

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

typedef HGLRC (WINAPI wgl_create_context_attribs_arb)(HDC DC, HGLRC ShareContext, const int *Attribs);
wgl_create_context_attribs_arb *wglCreateContextAttribsARB;

typedef BOOL (WINAPI wgl_choose_pixel_format_arb)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;

// TODO: Find out why this did not work...
// typedef HGLRC (wgl_create_context_attribs_arb(HDC DC,
//                                               HGLRC ShareContext,
//                                               const int *AttribList));
// wgl_create_context_attribs_arb *wglCreateContextAttribsARB;

struct opengl_information
{
    char *Vendor;
    char *Renderer;
    char *Version;
    char *GLSLVersion;
    bool ModernContext;
    int ColorBits;
} OpenGLInfo;

struct vdb_window
{
    HWND Handle;
};

static void
vdb_initOpenGL(HWND Window)
{
    PIXELFORMATDESCRIPTOR DesiredFormat = {
        sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
        1,                     // version number
        PFD_DRAW_TO_WINDOW |   // support window
        PFD_SUPPORT_OPENGL |   // support OpenGL
        PFD_DOUBLEBUFFER,      // double buffered
        PFD_TYPE_RGBA,         // RGBA type
        32,                    // 32-bit color depth
        0, 0, 0, 0, 0, 0,      // color bits ignored
        8,                     // no alpha buffer
        0,                     // shift bit ignored
        0,                     // no accumulation buffer
        0, 0, 0, 0,            // accum bits ignored
        32,                    // 32-bit z-buffer
        0,                     // no stencil buffer
        0,                     // no auxiliary buffer
        PFD_MAIN_PLANE,        // main layer
        0,                     // reserved
        0, 0, 0                // layer masks ignored
    };

    HDC WindowDC = GetDC(Window);
    int GotFormatIndex = ChoosePixelFormat(WindowDC, &DesiredFormat);
    PIXELFORMATDESCRIPTOR GotFormat;
    DescribePixelFormat(WindowDC, GotFormatIndex,
                        sizeof(GotFormat), &GotFormat);
    SetPixelFormat(WindowDC, GotFormatIndex, &GotFormat);

    HGLRC OpenGLRC = wglCreateContext(WindowDC);
    if (!wglMakeCurrent(WindowDC, OpenGLRC))
    {
        assert(false);
    }

    OpenGLInfo.ModernContext = false;

    #if 0
    wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb*)wglGetProcAddress("wglChoosePixelFormatARB");

    if (wglChoosePixelFormatARB)
    {
        // glEnable(GL_MULTISAMPLE)
        int GotFormatIndex;
        UINT MaxFormats = 1;
        UINT ReturnedFormats;
        int AttribList[] = {

        };
        wglChoosePixelFormatARB(WindowDC, IAttribList, 0, MaxFormats, &GotFormatIndex, &ReturnedFormats);

        PIXELFORMATDESCRIPTOR GotFormat;
        DescribePixelFormat(WindowDC, GotFormatIndex,
                            sizeof(GotFormat), &GotFormat);
        SetPixelFormat(WindowDC, GotFormatIndex, &GotFormat);
    }
    #endif

    wglCreateContextAttribsARB = (wgl_create_context_attribs_arb*)wglGetProcAddress("wglCreateContextAttribsARB");
    if (wglCreateContextAttribsARB)
    {
        HGLRC ShareContext = 0;
        int AttribList[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB,
            1,

            WGL_CONTEXT_MINOR_VERSION_ARB,
            5,

            WGL_CONTEXT_FLAGS_ARB,
            WGL_CONTEXT_DEBUG_BIT_ARB,

            WGL_CONTEXT_PROFILE_MASK_ARB,
            WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,

            0 // Terminate list
        };
        HGLRC NewOpenGLRC = wglCreateContextAttribsARB(WindowDC, ShareContext, AttribList);
        if (NewOpenGLRC && wglMakeCurrent(WindowDC, NewOpenGLRC))
        {
            wglDeleteContext(OpenGLRC);
            OpenGLRC = NewOpenGLRC;
            OpenGLInfo.ModernContext = true;
        }
        else
        {
            wglMakeCurrent(WindowDC, OpenGLRC);
        }
    }

    // Get OpenGL information
    {
        OpenGLInfo.ColorBits = GotFormat.cColorBits;
        OpenGLInfo.Vendor = (char*)glGetString(GL_VENDOR);
        OpenGLInfo.Renderer = (char*)glGetString(GL_RENDERER);
        OpenGLInfo.Version = (char*)glGetString(GL_VERSION);
        if (OpenGLInfo.ModernContext)
        {
            OpenGLInfo.GLSLVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        }
        else
        {
            OpenGLInfo.GLSLVersion = 0;
        }
        char *Extensions = (char*)glGetString(GL_EXTENSIONS);
        #if 0
        printf("Extensions: %s\n", Extensions);
        printf("Vendor: %s\n", OpenGLInfo.Vendor);
        printf("Renderer: %s\n", OpenGLInfo.Renderer);
        printf("Version: %s\n", OpenGLInfo.Version);
        printf("GLSLVersion: %s\n", OpenGLInfo.GLSLVersion);
        printf("Color bits: %d\n", OpenGLInfo.ColorBits);
        #endif
    }

    ReleaseDC(Window, WindowDC);

    wglSwapIntervalEXT = (wgl_swap_interval_ext)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(1);
    }
}

static LRESULT CALLBACK
vdb_windowProc(HWND   Window,
               UINT   Message,
               WPARAM WParam,
               LPARAM LParam)
{
    switch (Message)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        } break;
    }
    return DefWindowProcW(Window, Message, WParam, LParam);
}

vdb_window
vdb_openWindow(int WindowWidth, int WindowHeight)
{
    HINSTANCE Instance = GetModuleHandle(0);
    WNDCLASSW WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = vdb_windowProc;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = L"vdb_WindowClass";

    if (!RegisterClassW(&WindowClass))
    {
        // Failed to register window
        assert(false);
    }

    wchar_t *WindowName = L"vdb";
    DWORD WindowFlags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    int WindowInitX = CW_USEDEFAULT;
    int WindowInitY = CW_USEDEFAULT;
    HWND WindowHandle = CreateWindowExW(
        0,
        WindowClass.lpszClassName,
        WindowName,
        WindowFlags,
        WindowInitX,
        WindowInitY,
        WindowWidth,
        WindowHeight,
        0,
        0,
        Instance,
        0);

    if (!WindowHandle)
    {
        // Failed to open window
        assert(false);
    }

    vdb_initOpenGL(WindowHandle);

    // Increase timer resolution. I think Windows automatically resets the
    // period when the process closes.
    timeBeginPeriod(1);

    vdb_window Result = {0};
    Result.Handle = WindowHandle;
    return Result;
}

static int vdb_charCountOfUTF8String(const char *Text)
{
    int CharCount = 0;
    const vdb_u08 *ByteIndex = (const vdb_u08*)Text;
    while (*ByteIndex)
    {
        vdb_u08 UpperTwoBits = *ByteIndex & 0xC0;
        if (UpperTwoBits != 0x80)
        {
            CharCount++;
        }
        ByteIndex++;
    }
    return CharCount;
}

// Returns a utf-8 encoded and null terminated string
const char *vdb_getClipboardText()
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
void vdb_setClipboardText(const char *Text)
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
        int CharCount = vdb_charCountOfUTF8String(Text);
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

void vdb_imgui_processMessage(MSG Message)
{
    ImGuiIO &IO = ImGui::GetIO();
    switch (Message.message)
    {
        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            int vk = Message.wParam;
            IO.KeysDown[vk] = (Message.message == WM_KEYDOWN);
            IO.KeyShift = (GetKeyState(VK_SHIFT) & 128) != 0;
            IO.KeyCtrl = (GetKeyState(VK_CONTROL) & 128) != 0;
            IO.KeyAlt = (GetKeyState(VK_MENU) & 128) != 0;
        } break;

        case WM_CHAR:
        {
            IO.AddInputCharacter(Message.wParam);
        } break;

        case WM_MOUSEWHEEL:
        {
            int WheelDelta = GET_WHEEL_DELTA_WPARAM(Message.wParam);
            if (WheelDelta > 0)
                IO.MouseWheel = +1.0f;
            else if (WheelDelta < 0)
                IO.MouseWheel = -1.0f;
        } break;

        case WM_MOUSEMOVE:
        {
            float X = (float)GET_X_LPARAM(Message.lParam);
            float Y = (float)GET_Y_LPARAM(Message.lParam);
            IO.MousePos = ImVec2(X, Y);
        } break;
    }
}

bool vdb_processMessages(vdb_window Window, vdb_input *Input, vdb_event *Event)
{
    // Hide OS mouse cursor if ImGui is drawing it
    // Should handle mouse tracking if this is enabled
    // ImGuiIO.MouseDrawCursor = true;
    // ShowCursor(false);

    // Reset character input events
    for (int AnsiCode = 0; AnsiCode < 256; AnsiCode++)
    {
        Input->Keys[AnsiCode] = false;
    }

    Input->Mouse.Left.WasDownThisFrame = false;
    Input->Mouse.Right.WasDownThisFrame = false;
    Input->Mouse.Middle.WasDownThisFrame = false;

    Event->StepOnce = false;
    Event->StepOver = false;
    Event->Exit = false;

    MSG Message;
    while (PeekMessageW(&Message, 0, 0, 0, PM_REMOVE))
    {
        vdb_imgui_processMessage(Message);
        switch (Message.message)
        {
            case WM_QUIT:
            {
                Event->Exit = true;
                return false;
            } break;

            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                if (Message.wParam == VK_ESCAPE)
                    PostQuitMessage(0);
                if (Message.wParam == VK_F10)
                    Event->StepOnce = true;
                if (Message.wParam == VK_F5)
                    Event->StepOver = true;
                TranslateMessage(&Message);
            } break;

            case WM_CHAR:
            {
                if (Message.wParam < 256)
                    Input->Keys[Message.wParam] = true;
            } break;

            case WM_LBUTTONDOWN: Input->Mouse.Left.IsDown   = true; Input->Mouse.Left.WasDownThisFrame = true; break;
            case WM_RBUTTONDOWN: Input->Mouse.Right.IsDown  = true; Input->Mouse.Right.WasDownThisFrame = true; break;
            case WM_MBUTTONDOWN: Input->Mouse.Middle.IsDown = true; Input->Mouse.Middle.WasDownThisFrame = true; break;

            case WM_LBUTTONUP:   Input->Mouse.Left.IsDown   = false; break;
            case WM_RBUTTONUP:   Input->Mouse.Right.IsDown  = false; break;
            case WM_MBUTTONUP:   Input->Mouse.Middle.IsDown = false; break;

            case WM_MOUSEMOVE:
            {
                int X = GET_X_LPARAM(Message.lParam);
                int Y = GET_Y_LPARAM(Message.lParam);
                Input->Mouse.X = (float)X;
                Input->Mouse.Y = (float)Y;
            } break;

            default:
            {
                DispatchMessageW(&Message);
            }
        }
    }

    RECT ClientRect;
    GetClientRect(Window.Handle, &ClientRect);
    Input->WindowWidth = ClientRect.right-ClientRect.left;
    Input->WindowHeight = ClientRect.bottom-ClientRect.top;

    ImGuiIO &IO = ImGui::GetIO();
    {
        IO.DeltaTime = Input->DeltaTime > 0.0f ? Input->DeltaTime : 1.0f/60.0f;
        IO.DisplaySize = ImVec2((float)Input->WindowWidth, (float)Input->WindowHeight);

        // We might process both a BUTTONDOWN and BUTTONUP signal in the same frame,
        // but we want a down signal to always persist atleast one frame.
        IO.MouseDown[0] = Input->Mouse.Left.WasDownThisFrame || Input->Mouse.Left.IsDown;
        IO.MouseDown[1] = Input->Mouse.Right.WasDownThisFrame || Input->Mouse.Right.IsDown;
        IO.MouseDown[2] = Input->Mouse.Middle.WasDownThisFrame || Input->Mouse.Middle.IsDown;
    }

    return true;
}

void vdb_swapBuffers(vdb_window Window)
{
    HDC DeviceContext = GetDC(Window.Handle);
    SwapBuffers(DeviceContext);
    ReleaseDC(Window.Handle, DeviceContext);
}

void vdb_sleep(int milliseconds)
{
    Sleep(milliseconds);
}

void vdb_exit()
{
    exit(1);
}
