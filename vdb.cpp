#include "vdb.h"
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_opengl.h>
#include <SDL_assert.h>
#include "lib/imgui/imgui_draw.cpp"
#include "lib/imgui/imgui.cpp"
#include "lib/imgui/imgui_demo.cpp"
#include "lib/imgui/imgui_impl_sdl.cpp"
#include "lib/so_math.h"

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
    bool TakeNote;
    bool TakeScreenshot;
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
    #ifdef VDB_MY_CONFIG
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
    #else
    Result.Width = 640;
    Result.Height = 480;
    Result.PositionX = SDL_WINDOWPOS_UNDEFINED;
    Result.PositionY = SDL_WINDOWPOS_UNDEFINED;
    #endif
    return Result;
}

void vdb_saveSettings(vdb_settings Settings)
{
    #ifdef VDB_MY_CONFIG
    FILE *File = fopen(VDB_SETTINGS_FILENAME, "wb+");
    if (File)
    {
        fwrite((const void*)&Settings, sizeof(vdb_settings), 1, File);
        fclose(File);
    }
    #endif
}

vdb_window vdb_openWindow()
{
    vdb_window Result = {0};
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0); // @ Find out why this stopped working
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

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
    InterestingEvents->StepOver = false;
    InterestingEvents->TakeNote = false;
    InterestingEvents->TakeScreenshot = false;
    InterestingEvents->ReturnPressed = false;

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
            } break;

            case SDL_KEYUP:
            {
                if (Event.key.keysym.scancode == SDL_SCANCODE_F5)
                    InterestingEvents->StepOver = true;
                if (Event.key.keysym.scancode == SDL_SCANCODE_F10)
                    InterestingEvents->StepOnce = true;
                if (Event.key.keysym.scancode == SDL_SCANCODE_F12)
                    InterestingEvents->TakeNote = true;
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

void vdb(char *Label, vdb_callback Callback)
{
    static char *PrevLabel = 0;
    static bool Initialized = false;
    static bool StepOver = false;
    static vdb_window Window = {0};
    if (!Initialized)
    {
        Window = vdb_openWindow();
        ImGui_ImplSdl_Init(Window.SDLWindow);
        Initialized = true;
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
    float MainGuiAlpha = 1.0f;
    float MainMenuAlpha = 0.7f;
    bool Running = true;
    bool SaveScreenshot = false;
    char *ScreenshotFilename = 0;
    vdb_input Input = {0};
    vdb_event Event = {0};
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
        Input.Mouse.Y_NDC = -1.0f + 2.0f * Input.Mouse.Y / Input.WindowHeight;

        glViewport(0, 0, Input.WindowWidth, Input.WindowHeight);
        ImGui_ImplSdl_NewFrame(Window.SDLWindow);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
                            SaveScreenshot ? 0.0f : MainGuiAlpha);

        // Pre-callback state initialization
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glEnable(GL_POINT_SMOOTH);
            glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
            glPointSize(1.0f);

            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
            glLineWidth(1.0f);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            vdbOrtho(-1.0f, +1.0f, -1.0f, +1.0f);
        }
        Callback(Input);
        ImGui::PopStyleVar();

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
                            SaveScreenshot ? 0.0f : MainMenuAlpha);
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                if (ImGui::SliderFloat("Gui alpha", &MainGuiAlpha, 0.0f, 1.0f))
                {
                    if (MainGuiAlpha < 0.05f)
                        MainGuiAlpha = 0.0f;
                    if (MainGuiAlpha > 0.95f)
                        MainGuiAlpha = 1.0f;
                }
                #if 1
                static int UserWidth = Input.WindowWidth;
                static int UserHeight = Input.WindowHeight;
                ImGui::InputInt("Window width", &UserWidth, 1, 2560);
                ImGui::InputInt("Window height", &UserHeight, 1, 2560);
                if (ImGui::Button("Set window size"))
                {
                    SDL_SetWindowSize(Window.SDLWindow, UserWidth, UserHeight);
                }
                #endif
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debugger"))
            {
                if (ImGui::MenuItem("Step once [F10]"))
                    Event.StepOnce = true;
                if (ImGui::MenuItem("Step over [F5]"))
                    Event.StepOver = true;
                if (ImGui::MenuItem("Screenshot [PrtScreen]"))
                    Event.TakeScreenshot = true;
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar();

        if (SaveScreenshot)
        {
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

        if (Event.TakeScreenshot)
            ImGui::OpenPopup("Save screenshot as...");
        if (ImGui::BeginPopupModal("Save screenshot as...",
            NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char Filename[1024];
            ImGui::SetKeyboardFocusHere();
            ImGui::InputText("Filename", Filename, sizeof(Filename));
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120,0)) || Event.ReturnPressed)
            {
                SaveScreenshot = true;
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
