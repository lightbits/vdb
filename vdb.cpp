#include "vdb.h"
#include <stdint.h>

#define SDL_ASSERT_LEVEL 2
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_opengl.h>
#include <SDL_assert.h>
#include "lib/imgui/imgui_draw.cpp"
#include "lib/imgui/imgui.cpp"
#include "lib/imgui/imgui_demo.cpp"
#include "lib/imgui/imgui_impl_sdl.cpp"

#ifndef VDB_NO_MATH
#include "lib/so_math.h"
#endif

#ifndef VDB_NO_STB_IMAGE_WRITE
#ifdef STB_IMAGE_WRITE_IMPLEMENTATION
#error "Library conflict: Please #define VDB_NO_STB_IMAGE_WRITE before including vdb"
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"
#endif

#ifndef VDB_NO_STB_IMAGE
#ifdef STB_IMAGE_IMPLEMENTATION
#error "Library conflict: Please #define VDB_NO_STB_IMAGE before including vdb"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#endif

static bool VDB_FIRST_LOOP_ITERATION = false;

bool vdb_isFirstLoopIteration()
{
    return VDB_FIRST_LOOP_ITERATION;
}

#include "vdb_util.h"

struct vdb_window
{
    SDL_Window *SDLWindow;
    SDL_GLContext GLContext;
    bool HasVsync;
};

struct vdb_event
{
    bool StepOver;
    bool StepOnce;
    bool MarkSkip;
    bool TakeScreenshot;
    bool TakeScreenshotNoDialog;
    bool ReturnPressed;
    bool WindowSizeChanged;
    bool Exit;
};

struct vdb_settings
{
    int Width;
    int Height;
    int PositionX;
    int PositionY;
};

vdb_settings vdb_loadSettings()
{
    vdb_settings Result = {0};
    FILE *File = fopen(VDB_SETTINGS_FILENAME, "rb");
    if (File)
    {
        char Buffer[sizeof(vdb_settings)];
        size_t Count = fread(Buffer, sizeof(vdb_settings), 1, File);
        if (Count == 1)
        {
            Result = *(vdb_settings*)Buffer;
        }
        fclose(File);
    }
    else
    {
        Result.Width = 640;
        Result.Height = 480;
        Result.PositionX = SDL_WINDOWPOS_UNDEFINED;
        Result.PositionY = SDL_WINDOWPOS_UNDEFINED;
    }
    return Result;
}

void vdb_saveSettings(vdb_settings Settings)
{
    FILE *File = fopen(VDB_SETTINGS_FILENAME, "wb+");
    if (File)
    {
        fwrite((const void*)&Settings, sizeof(vdb_settings), 1, File);
        fclose(File);
    }
}

vdb_window vdb_openWindow()
{
    vdb_window Result = {0};

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        SDL_assert(false);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, VDB_GL_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, VDB_GL_MINOR);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, VDB_MULTISAMPLES > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, VDB_MULTISAMPLES);

    vdb_settings Settings = vdb_loadSettings();
    Result.SDLWindow = SDL_CreateWindow(
        "vdb",
        Settings.PositionX,
        Settings.PositionY,
        Settings.Width,
        Settings.Height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_assert(Result.SDLWindow);

    Result.GLContext = SDL_GL_CreateContext(Result.SDLWindow);
    SDL_assert(Result.GLContext != 0);

    // 0 for immediate updates, 1 for updates synchronized with the
    // vertical retrace. If the system supports it, you may
    // specify -1 to allow late swaps to happen immediately
    // instead of waiting for the next retrace.
    SDL_GL_SetSwapInterval(1);

    // Instead of using vsync, you can specify a desired framerate
    // that the application will attempt to keep. If a frame rendered
    // too fast, it will sleep the remaining time. Leave swap_interval
    // at 0 when using this.
    if (SDL_GL_GetSwapInterval() == 1)
        Result.HasVsync = true;
    else
        Result.HasVsync = false;

    return Result;
}

void vdb_processMessages(vdb_input *Input,
                         vdb_event *InterestingEvents)
{
    Input->Mouse.Left.Released = false;
    Input->Mouse.Right.Released = false;
    Input->Mouse.Middle.Released = false;
    InterestingEvents->WindowSizeChanged = false;
    InterestingEvents->Exit = false;
    InterestingEvents->StepOnce = false;
    InterestingEvents->MarkSkip = false;
    InterestingEvents->StepOver = false;
    InterestingEvents->TakeScreenshot = false;
    InterestingEvents->TakeScreenshotNoDialog = false;
    InterestingEvents->ReturnPressed = false;

    for (int Scancode = 0; Scancode < SDL_NUM_SCANCODES; Scancode++)
    {
        Input->Keys[Scancode].Released = false;
        Input->Keys[Scancode].Pressed = false;
    }

    SDL_Event Event;
    while (SDL_PollEvent(&Event))
    {
        ImGui_ImplSdl_ProcessEvent(&Event);

        // TODO: Check if IMGUI wants to capture mouse/key
        switch (Event.type)
        {
            case SDL_WINDOWEVENT:
            {
                switch (Event.window.event)
                {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        InterestingEvents->WindowSizeChanged = true;
                    } break;
                }
            } break;

            case SDL_KEYDOWN:
            {
                if (Event.key.keysym.sym == SDLK_ESCAPE)
                    InterestingEvents->Exit = true;
                if (!Input->Keys[Event.key.keysym.scancode].Down)
                    Input->Keys[Event.key.keysym.scancode].Pressed = true;
                Input->Keys[Event.key.keysym.scancode].Down = true;
            } break;

            case SDL_KEYUP:
            {
                if (Input->Keys[Event.key.keysym.scancode].Down)
                    Input->Keys[Event.key.keysym.scancode].Released = true;
                Input->Keys[Event.key.keysym.scancode].Down = false;
                if (Event.key.keysym.scancode == SDL_SCANCODE_F5)
                    InterestingEvents->StepOver = true;
                if (Event.key.keysym.scancode == SDL_SCANCODE_F10)
                    InterestingEvents->StepOnce = true;
                if (Event.key.keysym.scancode == SDL_SCANCODE_F10 &&
                    Event.key.keysym.mod & KMOD_LSHIFT)
                    InterestingEvents->MarkSkip = true;
                if (Event.key.keysym.scancode == SDL_SCANCODE_PRINTSCREEN)
                    InterestingEvents->TakeScreenshot = true;
                if (Event.key.keysym.scancode == SDL_SCANCODE_RETURN)
                    InterestingEvents->ReturnPressed = true;
                break;
            }

            case SDL_QUIT:
            {
                InterestingEvents->Exit = true;
            } break;

            case SDL_MOUSEBUTTONDOWN:
            {
                if (SDL_BUTTON_LMASK & Event.button.button)
                    Input->Mouse.Left.Down = true;
                if (SDL_BUTTON_RMASK & Event.button.button)
                    Input->Mouse.Right.Down = true;
                if (SDL_BUTTON_MMASK & Event.button.button)
                    Input->Mouse.Middle.Down = true;
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if (SDL_BUTTON_LMASK & Event.button.button)
                {
                    Input->Mouse.Left.Down = false;
                    Input->Mouse.Left.Released = true;
                }
                if (SDL_BUTTON_RMASK & Event.button.button)
                {
                    Input->Mouse.Right.Down = false;
                    Input->Mouse.Right.Released = true;
                }
                if (SDL_BUTTON_MMASK & Event.button.button)
                {
                    Input->Mouse.Middle.Down = false;
                    Input->Mouse.Middle.Released = true;
                }
                break;
            }
        }
    }
}

uint64_t vdb_getTicks()
{
    return SDL_GetPerformanceCounter();
}

float vdb_getElapsedSeconds(uint64_t Begin, uint64_t End)
{
    uint64_t Frequency = SDL_GetPerformanceFrequency();
    return (float)(End-Begin) / (float)Frequency;
}

#define COROUTINE(Trigger, Duration) \
    static uint64_t TickBegin_##__LINE__ = 0; \
    if (Trigger) TickBegin_##__LINE__ = vdb_getTicks(); \
    if (vdb_getElapsedSeconds(TickBegin_##__LINE__, vdb_getTicks()) < Duration) \

void vdbs(const char *Label, vdb_callback Callback) { }

void vdb(const char *Label, vdb_callback Callback)
{
    struct watch_window
    {
        const char *Label;
        bool Skip;
    };

    static watch_window WatchWindows[1024];
    static int WatchWindowCount = 0;

    static const char *PrevLabel = 0;
    static bool Initialized = false;
    static bool StepOver = false;
    static vdb_window Window = {0};
    if (!Initialized)
    {
        Window = vdb_openWindow();
        ImGui_ImplSdl_Init(Window.SDLWindow);
        Initialized = true;
        WatchWindowCount = 0;
    }

    // Add new watch window entry if this was a new one
    // If it wasn't new, check if the user wants to skip it
    int WatchWindowIndex = 0;
    {
        int Index = 0;
        bool Added = false;
        while (Index < WatchWindowCount)
        {
            if (WatchWindows[Index].Label == Label)
            {
                Added = true;
                break;
            }
            Index++;
        }
        if (!Added && WatchWindowCount < 1024)
        {
            Index = WatchWindowCount;
            WatchWindows[Index].Label = Label;
            WatchWindows[Index].Skip = false;
            WatchWindowCount++;
        }
        if (WatchWindows[Index].Skip)
        {
            return;
        }
        WatchWindowIndex = Index;
    }

    // @ maybe store the callback addresses?
    // recall

    bool IsNewCallback = false;
    if (PrevLabel != Label)
    {
        PrevLabel = Label;
        IsNewCallback = true;
    }

    if (StepOver)
    {
        if (IsNewCallback)
        {
            StepOver = false;
        }
        else
        {
            return;
        }
    }

    uint64_t StartTick = vdb_getTicks();
    uint64_t LastTick = StartTick;
    float MainMenuAlpha = 0.7f;
    bool MainMenuActive = false;
    bool Running = true;

    #ifdef VDB_IMGUI_CURSOR
    ImGui::GetIO().MouseDrawCursor = true;
    #endif

    bool ImGuiCursor = ImGui::GetIO().MouseDrawCursor;

    bool        SaveScreenshot = false;
    static bool ScreenshotDrawCursor = false;
    static bool ScreenshotDrawGui = false;
    int         ScreenshotCursorX = 0;
    int         ScreenshotCursorY = 0;
    char       *ScreenshotFilename = 0;

    vdb_input Input = {0};
    vdb_event Event = {0};

    static vdb_input::key KeyStates[SDL_NUM_SCANCODES];
    Input.Keys = KeyStates;

    Input.ScreenshotDrawCursor = &ScreenshotDrawCursor;
    Input.ScreenshotDrawGui = &ScreenshotDrawGui;
    Input.ScreenshotFilename = &ScreenshotFilename;
    Input.TakeScreenshotNoDialog = &Event.TakeScreenshotNoDialog;
    Input.StepOnce = &Event.StepOnce;

    VDB_FIRST_LOOP_ITERATION = true;
    while (Running)
    {
        uint64_t CurrTick = vdb_getTicks();
        Input.DeltaTime = vdb_getElapsedSeconds(LastTick, CurrTick);
        Input.ElapsedTime = vdb_getElapsedSeconds(StartTick, CurrTick);
        LastTick = CurrTick;

        vdb_processMessages(&Input, &Event);

        SDL_GetWindowSize(Window.SDLWindow, &Input.WindowWidth, &Input.WindowHeight);
        SDL_GetMouseState(&Input.Mouse.X, &Input.Mouse.Y);
        Input.Mouse.X_NDC = -1.0f + 2.0f * Input.Mouse.X / Input.WindowWidth;
        Input.Mouse.Y_NDC = -1.0f + 2.0f * (Input.WindowHeight-1-Input.Mouse.Y) / Input.WindowHeight;

        glViewport(0, 0, Input.WindowWidth, Input.WindowHeight);

        if (SaveScreenshot)
        {
            if (!ScreenshotDrawGui)
            {
                // Setting this instead of MainGuiAlpha because unnamed windows partially remain even though their alpha is zero
                ImGui::GetStyle().Alpha = 0.0f;
                ImGui::GetIO().MouseDrawCursor = false;
            }
            else
            {
                if (!ScreenshotDrawCursor)
                {
                    ImGui::GetIO().MouseDrawCursor = false;
                }
            }
        }
        else
        {
            ImGui::GetStyle().Alpha = 1.0f;
            if (ImGuiCursor)
                ImGui::GetIO().MouseDrawCursor = true;
        }
        ImGui_ImplSdl_NewFrame(Window.SDLWindow);

        // Pre-callback state initialization
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClearDepth(1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDisable(GL_DEPTH_TEST);

            glEnable(GL_POINT_SMOOTH);
            glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

            glPointSize(1.0f);
            glLineWidth(1.0f);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Specify the start of each pixel row in memory to be 1-byte aligned
            // as opposed to 4-byte aligned or something.
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            vdbOrtho(-1.0f, +1.0f, -1.0f, +1.0f);
        }

        Callback(Input);

        if (Input.Mouse.Y < 30 || MainMenuActive)
            MainMenuAlpha = 0.7f;
        else
            MainMenuAlpha = 0.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, MainMenuAlpha);
        if (ImGui::BeginMainMenuBar())
        {
            MainMenuActive = false;
            if (ImGui::BeginMenu("Options"))
            {
                MainMenuActive = true;
                static int UserWidth = Input.WindowWidth;
                static int UserHeight = Input.WindowHeight;
                ImGui::InputInt("Window width", &UserWidth, 1, 2560);
                ImGui::InputInt("Window height", &UserHeight, 1, 2560);
                if (ImGui::Button("Set window size"))
                {
                    SDL_SetWindowSize(Window.SDLWindow, UserWidth, UserHeight);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debugger"))
            {
                MainMenuActive = true;
                if (ImGui::MenuItem("Step once [F10]"))
                    Event.StepOnce = true;
                if (ImGui::MenuItem("Step over [F5]"))
                    Event.StepOver = true;
                if (ImGui::MenuItem("Screenshot [PrtScreen]"))
                    Event.TakeScreenshot = true;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Watches"))
            {
                MainMenuActive = true;
                for (int i = 0; i < WatchWindowCount; i++)
                {
                    ImGui::Checkbox(WatchWindows[i].Label, &WatchWindows[i].Skip);
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar();

        if (SaveScreenshot && ScreenshotDrawCursor && !ScreenshotDrawGui)
        {
            glDisable(GL_LINE_SMOOTH);
            glLineWidth(1.0f);

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

            int X = ScreenshotCursorX;
            int Y = ScreenshotCursorY;
            vdbOrtho(0.0f, Input.WindowWidth, Input.WindowHeight, 0.0f);
            glBegin(GL_LINES);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glVertex2f(X-8.0f, Y);
            glVertex2f(X+8.0f, Y);
            glVertex2f(X, Y-8.0f);
            glVertex2f(X, Y+8.0f);
            glEnd();
        }

        bool BeginSaveScreenshot = false;
        if (Event.TakeScreenshot)
        {
            ImGui::OpenPopup("Save screenshot as...");
            ScreenshotCursorX = Input.Mouse.X;
            ScreenshotCursorY = Input.Mouse.Y;
        }
        if (Event.TakeScreenshotNoDialog)
        {
            BeginSaveScreenshot = true;
            ScreenshotCursorX = Input.Mouse.X;
            ScreenshotCursorY = Input.Mouse.Y;
        }
        if (ImGui::BeginPopupModal("Save screenshot as...",
            NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char Filename[1024];
            COROUTINE(Event.TakeScreenshot, 1.0f)
            {
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::InputText("Filename", Filename, sizeof(Filename), ImGuiInputTextFlags_AutoSelectAll);
            ImGui::Separator();

            ImGui::Checkbox("Draw GUI", &ScreenshotDrawGui);
            ImGui::Checkbox("Draw crosshair", &ScreenshotDrawCursor);

            if (ImGui::Button("OK", ImVec2(120,0)) || Event.ReturnPressed)
            {
                BeginSaveScreenshot = true;
                ScreenshotFilename = Filename;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120,0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Render();

        if (SaveScreenshot)
        {
            SDL_assert(ScreenshotFilename);

            int Width = Input.WindowWidth;
            int Height = Input.WindowHeight;
            uint8_t *Pixels = (uint8_t*)calloc(Width*Height,3);

            // Read the back buffer. Note: I set the pack alignment to 1
            // before reading to avoid corruption.
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, Pixels);
            // Write result flipped vertically starting from the beginning of
            // the last row, and moving backward.
            uint8_t *End = Pixels + Width*Height*3 - Width*3;
            stbi_write_png(ScreenshotFilename, Width, Height, 3, End, -Width*3);
            free(Pixels);
            SaveScreenshot = false;
        }

        if (BeginSaveScreenshot)
        {
            SaveScreenshot = true;
            BeginSaveScreenshot = false;
        }

        SDL_GL_SwapWindow(Window.SDLWindow);

        if (!Window.HasVsync && Input.DeltaTime < 1.0f/60.0f)
        {
            float SleepSec = (1.0f/60.0f) - Input.DeltaTime;
            uint32_t SleepMs = (uint32_t)(SleepSec*1000.0f);
            SDL_Delay(SleepMs);
        }

        GLenum Error = glGetError();
        SDL_assert(Error == GL_NO_ERROR);

        VDB_FIRST_LOOP_ITERATION = false;

        if (Event.MarkSkip)
        {
            WatchWindows[WatchWindowIndex].Skip = true;
        }
        if (Event.Exit)
        {
            break;
        }
        if (Event.StepOnce)
        {
            break;
        }
        if (Event.StepOver)
        {
            StepOver = true;
            break;
        }
    }

    if (Event.Exit)
    {
        vdb_settings Settings = {0};
        Settings.Width = 0,
        Settings.Height = 0;
        Settings.PositionX = 0;
        Settings.PositionY = 0;
        SDL_GetWindowSize(Window.SDLWindow, &Settings.Width, &Settings.Height);
        SDL_GetWindowPosition(Window.SDLWindow, &Settings.PositionX, &Settings.PositionY);
        vdb_saveSettings(Settings);
        ImGui_ImplSdl_Shutdown();
        exit(0);
    }
}
