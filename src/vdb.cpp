#define assert_LEVEL 2
#include "SDL.h"
#include "glad/glad.c"
#ifdef _WIN32
#include <winuser.h> // for Windows' SetWindowPos (allows you to set topmost)
#undef WIN32_LEAN_AND_MEAN // defined by glad
#endif
#include "SDL_syswm.h"
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_demo.cpp"
#include "imgui/imgui_impl_sdl_gl3.h"
#include "imgui/imgui_impl_sdl_gl3.cpp"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "vdb.h"
#include "vdbconfig.h"
#include "_gl_state.cpp"
#include "_gl_error.cpp"
#include "_settings.cpp"
#include "_framegrab.cpp"
#include "_source_sans_pro.cpp"
#include "_texture.cpp"
#include "_shader.cpp"
#include "_sketch_mode.cpp"
#include "_ruler_mode.cpp"
#include "_render_texture.cpp"

#define HOTKEY_FRAMEGRAB   vdb.key_pressed[SDL_SCANCODE_S] && vdb.key_down[SDL_SCANCODE_LALT]
#define HOTKEY_WINDOW_SIZE vdb.key_pressed[SDL_SCANCODE_W] && vdb.key_down[SDL_SCANCODE_LALT]
#define HOTKEY_SKETCH_MODE vdb.key_pressed[SDL_SCANCODE_D] && vdb.key_down[SDL_SCANCODE_LALT]
#define HOTKEY_RULER_MODE  vdb.key_pressed[SDL_SCANCODE_R] && vdb.key_down[SDL_SCANCODE_LALT]

struct vdbGlobals
{
    const char *active_label;
    bool initialized;
    bool has_vsync;
    SDL_Window *window;
    SDL_GLContext context;

    // Note: for retina displays screen coordinates != framebuffer coordinates
    // This is the size in pixels of the framebuffer in the window
    int framebuffer_width;
    int framebuffer_height;
    // This is the size of the window's client area in screen coordinates
    int window_width;
    int window_height;
    // This is the position of the window's client area in screen coordinates
    int window_x;
    int window_y;
    int viewport_left;
    int viewport_bottom;
    int viewport_width;
    int viewport_height;
    bool key_pressed[SDL_NUM_SCANCODES];
    bool key_down[SDL_NUM_SCANCODES];
    bool key_released[SDL_NUM_SCANCODES];
    struct mouse_t
    {
        int x,y; // The position of the mouse in the client area in screen coordinates where (0,0):top-left
        vdbVec2 ndc; // -||- in normalized device coordinates where (-1,-1):bottom-left (+1,+1):top-right
        float wheel;
        struct button_t
        {
            bool pressed,released,down;
        } left,right,middle;
    } mouse;
    bool should_quit;
    bool escape_eaten;
    int note_index;
    bool is_first_frame;
    bool taa_begun;
    bool tss_begun;
    bool sketch_mode_active;
    bool ruler_mode_active;
    render_texture_t *current_render_texture;
    vdb_settings_t settings;
};
static vdbGlobals vdb = {0};

#include "vdb_render_texture.cpp"
#include "vdb_transform.cpp"
#include "vdb_camera.cpp"
#include "vdb_immediate.cpp"
#include "vdb_immediate_util.cpp"
#include "vdb_keyboard.cpp"
#include "vdb_mouse.cpp"
#include "vdb_shader.cpp"
#include "vdb_points.cpp"
#include "vdb_image.cpp"
#include "vdb_filter.cpp"

static void vdbCloseWindow();
static void vdbSetWindowSize(int width, int height, bool topmost);
static void vdbOpenWindow(int x, int y, int width, int height);
static void vdbSwapBuffers(float dt);
static void vdbPollEvents();
static void vdbExitDialog();
static void vdbSizeDialog();
static void vdbFramegrabDialog();

bool vdbIsFirstFrame() { return vdb.is_first_frame; }

void vdbDetachGLContext()
{
    assert(vdb.window);
    SDL_GL_MakeCurrent(vdb.window, NULL);
}

bool vdbBeginFrame(const char *label)
{
    static const char *skip_label = NULL;
    // static const char *prev_label = NULL;
    static bool is_first_frame = true;
    vdb.is_first_frame = is_first_frame;
    // prev_label = label;
    if (skip_label == label)
        return false;
    is_first_frame = false; // todo: first frame detection is janky.
                            // consider e.g. a for loop with single-stepping

    if (!vdb.initialized)
    {
        vdb.settings = vdbLoadSettingsOrDefault(VDB_SETTINGS_FILENAME);
        vdbOpenWindow(vdb.settings.window_x, vdb.settings.window_y, vdb.settings.window_w, vdb.settings.window_h);
        ImGui::CreateContext();
        ImGui_ImplSdlGL3_Init(vdb.window);
        ImGui::StyleColorsDark();
        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF((const char*)source_sans_pro_compressed_data, source_sans_pro_compressed_size, VDB_FONT_HEIGHT);
        ImGui::GetStyle().WindowBorderSize = 0.0f;
        vdb.initialized = true;

        // Assuming user uploads images that are one-byte packed?
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        CheckGLError();
    }

    SDL_GLContext current = SDL_GL_GetCurrentContext();
    if (current != vdb.context)
        SDL_GL_MakeCurrent(vdb.window, vdb.context);

    vdbPollEvents();

    static int save_settings_counter = 60;
    if (save_settings_counter >= 0)
        save_settings_counter--;
    if (vdb.settings.window_x != vdb.window_x ||
        vdb.settings.window_y != vdb.window_y ||
        vdb.settings.window_w != vdb.window_width ||
        vdb.settings.window_h != vdb.window_height)
    {
        vdb.settings.window_x = vdb.window_x;
        vdb.settings.window_y = vdb.window_y;
        vdb.settings.window_w = vdb.window_width;
        vdb.settings.window_h = vdb.window_height;
        // if the settings changed and we haven't saved in a while (to
        // spare disk usage) then we save to disk.
        if (save_settings_counter < 0)
        {
            save_settings_counter = 60;
            vdbSaveSettings(vdb.settings, VDB_SETTINGS_FILENAME);
        }
    }

    if (vdb.key_pressed[SDL_SCANCODE_F10]) { vdbSaveSettings(vdb.settings, VDB_SETTINGS_FILENAME); is_first_frame = true; return false; }
    if (vdb.key_pressed[SDL_SCANCODE_F5]) { vdbSaveSettings(vdb.settings, VDB_SETTINGS_FILENAME); is_first_frame = true; skip_label = label; return false; }
    if (vdb.should_quit) { vdbSaveSettings(vdb.settings, VDB_SETTINGS_FILENAME); vdbCloseWindow(); exit(0); }

    vdbResetTransform();
    vdbResetMouseOverState();
    vdb.escape_eaten = false;
    vdb.note_index = 0;
    vdb.viewport_left = 0;
    vdb.viewport_bottom = 0;
    vdb.viewport_width = vdb.framebuffer_width;
    vdb.viewport_height = vdb.framebuffer_height;
    vdb.mouse.ndc = vdbWindowToNDC((float)vdb.mouse.x, (float)vdb.mouse.y);

    ImGui_ImplSdlGL3_NewFrame(vdb.window);
    CheckGLError();

    if (HOTKEY_SKETCH_MODE) vdb.sketch_mode_active = !vdb.sketch_mode_active;

    if (vdb.sketch_mode_active)
    {
        bool undo = vdbIsKeyDown(VDB_KEY_LCTRL) && vdbWasKeyPressed(VDB_KEY_Z);
        bool redo = vdbIsKeyDown(VDB_KEY_LCTRL) && vdbWasKeyPressed(VDB_KEY_Y);
        bool clear = vdbWasKeyPressed(VDB_KEY_D);
        bool click = vdbIsMouseLeftDown();
        bool brightness = vdbIsKeyDown(VDB_KEY_LALT);
        float x = vdbGetMousePos().x;
        float y = vdbGetMousePos().y;
        vdbSketchMode(undo, redo, clear, brightness, click, x, y);
        ImGui::GetIO().WantCaptureKeyboard = true;
        ImGui::GetIO().WantCaptureMouse = true;
    }

    if (HOTKEY_RULER_MODE) vdb.ruler_mode_active = !vdb.ruler_mode_active;

    if (vdb.ruler_mode_active)
    {
        if (vdb.key_pressed[SDL_SCANCODE_ESCAPE])
        {
            vdb.ruler_mode_active = false;
            vdb.escape_eaten = true;
        }
        vdbRulerMode(vdbIsMouseLeftDown(), vdbGetMousePos());
        ImGui::GetIO().WantCaptureKeyboard = true;
        ImGui::GetIO().WantCaptureMouse = true;
    }

    glDisable(GL_COLOR_LOGIC_OP);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glLineWidth(1.0f);
    glPointSize(1.0f);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, vdb.framebuffer_width, vdb.framebuffer_height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    CheckGLError();

    return true;
}

void vdbEndFrame()
{
    if (vdb.taa_begun) vdbEndTAA();
    if (vdb.tss_begun) vdbEndTSS();

    if (vdb.sketch_mode_active)
        vdbSketchModePresent();

    if (vdb.ruler_mode_active)
        vdbRulerModePresent();

    if (framegrab.active)
    {
        framegrab_options_t opt = framegrab.options;

        if (opt.draw_imgui)
        {
            ImGui::Render();
            ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
        }

        GLenum format = opt.alpha_channel ? GL_RGBA : GL_RGB;
        int channels = opt.alpha_channel ? 4 : 3;
        int width = vdb.framebuffer_width;
        int height = vdb.framebuffer_height;
        unsigned char *data = (unsigned char*)malloc(width*height*channels);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

        if (!opt.draw_imgui)
        {
            ImGui::Render();
            ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
        }

        FramegrabSaveOutput(data, width, height, channels, format);

        free(data);

        if (vdb.key_pressed[SDL_SCANCODE_ESCAPE])
        {
            StopFramegrab();
            vdb.escape_eaten = true;
        }
        else if (HOTKEY_FRAMEGRAB)
        {
            StopFramegrab();
        }
    }
    else
    {
        vdbSizeDialog();
        vdbFramegrabDialog();
        vdbExitDialog();
        ImGui::Render();
        ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
    }

    vdbSwapBuffers(1.0f/60.0f);
    CheckGLError();
}

static void vdbCloseWindow()
{
    SDL_GL_DeleteContext(vdb.context);
    SDL_DestroyWindow(vdb.window);
    SDL_Quit();
}

static void vdbSetWindowSize(SDL_Window *window, int w, int h, bool topmost)
{
    #ifdef _WIN32
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(window, &info))
    {
        HWND hwnd = info.info.win.window;
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = w;
        rect.bottom = h;
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
        int aw = rect.right-rect.left;
        int ah = rect.bottom-rect.top;
        if (topmost)
        {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, aw, ah, SWP_NOMOVE);
        }
        else
        {
            SetWindowPos(hwnd, HWND_TOP, 0, 0, aw, ah, SWP_NOMOVE);
        }
    }
    else
    {
        SDL_SetWindowSize(window, w, h);
    }
    #else
    SDL_SetWindowSize(window, w, h);
    #endif
}

static void vdbOpenWindow(int x, int y, int width, int height)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        assert(false);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, VDB_GL_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, VDB_GL_MINOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, VDB_ALPHABITS);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, VDB_DEPTHBITS);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, VDB_STENCILBITS);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, VDB_MULTISAMPLES > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, VDB_MULTISAMPLES);

    vdb.window = SDL_CreateWindow(
        "vdb",
        (x < 0) ? SDL_WINDOWPOS_CENTERED : x,
        (y < 0) ? SDL_WINDOWPOS_CENTERED : y,
        width,
        height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    assert(vdb.window);
    SDL_GL_LoadLibrary(NULL); // GLAD will do the loading for us after creating context
    vdb.context = SDL_GL_CreateContext(vdb.window);
    assert(vdb.context != 0);
    assert(gladLoadGLLoader(SDL_GL_GetProcAddress));
    assert(gladLoadGL());

    // 0 for immediate updates, 1 for updates synchronized with the
    // vertical retrace. If the system supports it, you may
    // specify -1 to allow late swaps to happen immediately
    // instead of waiting for the next retrace.
    SDL_GL_SetSwapInterval(1);

    // Instead of using vsync, you can specify a desired framerate
    // that the application will attempt to keep. If a frame rendered
    // too fast, it will sleep the remaining time. Leave swap_interval
    // at 0 when using this.
    vdb.has_vsync = (SDL_GL_GetSwapInterval() == 1);

    // printf("Vendor        : %s\n", glGetString(GL_VENDOR));
    // printf("Renderer      : %s\n", glGetString(GL_RENDERER));
    // printf("Version       : %s\n", glGetString(GL_VERSION));
    // printf("Extensions    : %s\n", glGetString(GL_EXTENSIONS));
    CheckGLError();
}

static void vdbSwapBuffers(float dt)
{
    SDL_GL_SwapWindow(vdb.window);
    if (!vdb.has_vsync && dt < 1.0f/60.0f)
    {
        float sleep_s = (1.0f/60.0f) - dt;
        Uint32 sleep_ms = (Uint32)(sleep_s*1000.0f);
        SDL_Delay(sleep_ms);
    }
}

static void vdbPollEvents()
{
    vdb.mouse.wheel = 0.0f;
    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        vdb.key_pressed[i] = false;
        vdb.key_released[i] = false;
    }
    vdb.mouse.left.pressed = false;
    vdb.mouse.right.pressed = false;
    vdb.mouse.middle.pressed = false;
    vdb.mouse.left.released = false;
    vdb.mouse.right.released = false;
    vdb.mouse.middle.released = false;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSdlGL3_ProcessEvent(&event);
        if (event.type == SDL_QUIT) vdb.should_quit = true;
        else
        if (event.type == SDL_KEYDOWN)
        {
            SDL_Scancode c = event.key.keysym.scancode;
            if (!vdb.key_down[c])
                vdb.key_pressed[c] = true;
            vdb.key_down[c] = true;
        }
        else
        if (event.type == SDL_KEYUP)
        {
            SDL_Scancode c = event.key.keysym.scancode;
            if (vdb.key_down[c])
                vdb.key_released[c] = true;
            vdb.key_down[c] = false;
        }
        else
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT) { if (!vdb.mouse.left.down) vdb.mouse.left.pressed = true; vdb.mouse.left.down = true; }
            if (event.button.button == SDL_BUTTON_RIGHT) { if (!vdb.mouse.right.down) vdb.mouse.right.pressed = true; vdb.mouse.right.down = true; }
            if (event.button.button == SDL_BUTTON_MIDDLE) { if (!vdb.mouse.middle.down) vdb.mouse.middle.pressed = true; vdb.mouse.middle.down = true; }
        }
        else
        if (event.type == SDL_MOUSEBUTTONUP)
        {
            if (event.button.button == SDL_BUTTON_LEFT) { if (vdb.mouse.left.down) vdb.mouse.left.released = true; vdb.mouse.left.down = false; }
            if (event.button.button == SDL_BUTTON_RIGHT) { if (vdb.mouse.right.down) vdb.mouse.right.released = true; vdb.mouse.right.down = false; }
            if (event.button.button == SDL_BUTTON_MIDDLE) { if (vdb.mouse.middle.down) vdb.mouse.middle.released = true; vdb.mouse.middle.down = false; }
        }
        else
        if (event.type == SDL_MOUSEWHEEL)
        {
            if (event.wheel.y > 0) vdb.mouse.wheel = +1.0f;
            else if (event.wheel.y < 0) vdb.mouse.wheel = -1.0f;
        }
    }

    SDL_GL_GetDrawableSize(vdb.window, &vdb.framebuffer_width, &vdb.framebuffer_height);
    SDL_GetWindowPosition(vdb.window, &vdb.window_x, &vdb.window_y);
    SDL_GetMouseState(&vdb.mouse.x, &vdb.mouse.y);
    SDL_GetWindowSize(vdb.window, &vdb.window_width, &vdb.window_height);
}

static void vdbExitDialog()
{
    bool escape = vdb.key_pressed[SDL_SCANCODE_ESCAPE];
    if (escape && !vdb.escape_eaten && vdb.settings.never_ask_on_exit)
    {
        vdb.should_quit = true;
        return;
    }
    if (escape && !vdb.escape_eaten && !vdb.settings.never_ask_on_exit)
    {
        ImGui::OpenPopup("Do you want to exit?##popup_exit");
    }
    if (ImGui::BeginPopupModal("Do you want to exit?##popup_exit", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::Button("Yes"))
        {
            vdb.should_quit = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Never ask me again", &vdb.settings.never_ask_on_exit);
        if (escape && !ImGui::IsWindowAppearing())
        {
            vdb.escape_eaten = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void vdbSizeDialog()
{
    if (HOTKEY_WINDOW_SIZE)
    {
        ImGui::OpenPopup("Set window size##popup");
        ImGui::CaptureKeyboardFromApp(true);
    }
    if (ImGui::BeginPopupModal("Set window size##popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static int width = 0, height = 0;
        if (ImGui::IsWindowAppearing())
        {
            width = vdb.window_width;
            height = vdb.window_height;
        }

        static bool topmost = false;
        ImGui::InputInt("Width", &width);
        ImGui::InputInt("Height", &height);
        ImGui::Separator();
        ImGui::Checkbox("Topmost", &topmost);

        if (ImGui::Button("OK", ImVec2(120,0)) || vdb.key_pressed[SDL_SCANCODE_RETURN])
        {
            vdbSetWindowSize(vdb.window, width, height, topmost);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120,0)))
        {
            ImGui::CloseCurrentPopup();
        }
        if (vdb.key_pressed[SDL_SCANCODE_ESCAPE])
        {
            ImGui::CloseCurrentPopup();
            vdb.escape_eaten = true;
        }
        ImGui::EndPopup();
    }
}

static void vdbFramegrabDialog()
{
    using namespace ImGui;
    bool enter_button = vdb.key_pressed[SDL_SCANCODE_RETURN];
    bool escape_button = vdb.key_pressed[SDL_SCANCODE_ESCAPE];
    if (HOTKEY_FRAMEGRAB)
    {
        OpenPopup("Take screenshot##popup");
        ImGui::CaptureKeyboardFromApp(true);
    }
    if (BeginPopupModal("Take screenshot##popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char filename[1024] = { 's','c','r','e','e','n','s','h','o','t','%','0','4','d','.','p','n','g',0 };
        if (IsWindowAppearing())
            SetKeyboardFocusHere();
        InputText("Filename", filename, sizeof(filename));

        static bool alpha = false;
        static int mode = 0;
        const int mode_single = 0;
        const int mode_sequence = 1;
        const int mode_ffmpeg = 2;
        static bool draw_imgui = false;
        static bool draw_cursor = false;
        RadioButton("Screenshot", &mode, mode_single);
        SameLine();
        ShowHelpMarker("Take a single screenshot. Put a %d in the filename to use the counter for successive screenshots.");
        SameLine();
        RadioButton("Sequence", &mode, mode_sequence);
        SameLine();
        ShowHelpMarker("Record a video of images in succession (e.g. output0000.png, output0001.png, ... etc.). Put a %d in the filename to get frame numbers. Use %0nd to left-pad with n zeroes.");
        SameLine();
        RadioButton("ffmpeg", &mode, mode_ffmpeg);
        SameLine();
        ShowHelpMarker("Record a video with raw frames piped directly to ffmpeg, and save the output in the format specified by your filename extension (e.g. mp4). This option can be quicker as it avoids writing to the disk.\nMake sure the 'ffmpeg' executable is visible from the terminal you launched this program in.");

        Checkbox("Alpha (32bpp)", &alpha);
        SameLine();
        Checkbox("Draw GUI", &draw_imgui);
        SameLine();
        Checkbox("Draw cursor", &draw_cursor);

        if (mode == mode_single)
        {
            static bool do_continue = true;
            static int start_from = 0;
            Checkbox("Continue counting", &do_continue);
            SameLine();
            ShowHelpMarker("Enable this to continue the image filename number suffix from the last screenshot captured (in this program session).");
            if (!do_continue)
            {
                SameLine();
                PushItemWidth(100.0f);
                InputInt("Start from", &start_from);
            }
            if (Button("OK", ImVec2(120,0)) || enter_button)
            {
                TakeScreenshot(filename, draw_imgui, draw_cursor, !do_continue, start_from, alpha);
                CloseCurrentPopup();
            }
            SameLine();
            if (Button("Cancel", ImVec2(120,0)))
            {
                CloseCurrentPopup();
            }
        }
        else if (mode == mode_sequence)
        {
            static bool do_continue = false;
            static int start_from = 0;
            static int frame_cap = 0;
            InputInt("Number of frames", &frame_cap);
            SameLine();
            ShowHelpMarker("0 for unlimited. To stop the recording at any time, press the same hotkey you used to open this dialog (CTRL+S by default).");

            Checkbox("Continue from last frame", &do_continue);
            SameLine();
            ShowHelpMarker("Enable this to continue the image filename number suffix from the last image sequence that was recording (in this program session).");
            if (!do_continue)
            {
                SameLine();
                PushItemWidth(100.0f);
                InputInt("Start from", &start_from);
            }

            if (Button("Start", ImVec2(120,0)) || enter_button)
            {
                RecordVideoToImageSequence(filename, frame_cap, draw_imgui, draw_cursor, !do_continue, start_from, alpha);
            }
            SameLine();
            ShowHelpMarker("Press ESCAPE or CTRL+S to stop.");
            SameLine();
            if (Button("Cancel", ImVec2(120,0)))
            {
                CloseCurrentPopup();
            }
        }
        else if (mode == mode_ffmpeg)
        {
            static int frame_cap = 0;
            static float framerate = 60;
            static int crf = 21;
            InputInt("Number of frames", &frame_cap);
            SameLine();
            ShowHelpMarker("0 for unlimited. To stop the recording at any time, press the same hotkey you used to open this dialog (CTRL+S by default).");
            SliderInt("Quality (lower is better)", &crf, 1, 51);
            InputFloat("Framerate", &framerate);

            if (Button("Start", ImVec2(120,0)) || enter_button)
            {
                RecordVideoToFfmpeg(filename, framerate, crf, frame_cap, draw_imgui, draw_cursor, alpha);
            }
            SameLine();
            ShowHelpMarker("Press ESCAPE or CTRL+S to stop.");
            SameLine();
            if (Button("Cancel", ImVec2(120,0)))
            {
                CloseCurrentPopup();
            }
        }

        if (escape_button)
        {
            CloseCurrentPopup();
            vdb.escape_eaten = true;
        }
        EndPopup();
    }
}
