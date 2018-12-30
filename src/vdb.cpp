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
#include "vdb_config.h"
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

namespace mouse
{
    static int x,y; // The position of the mouse in the client area in screen coordinates where (0,0):top-left
    static vdbVec2 ndc; // -||- in normalized device coordinates where (-1,-1):bottom-left (+1,+1):top-right
    static float wheel;
    static struct button_t
    {
        bool pressed,released,down;
    } left,right,middle;
}

namespace keys
{
    static bool pressed[SDL_NUM_SCANCODES];
    static bool down[SDL_NUM_SCANCODES];
    static bool released[SDL_NUM_SCANCODES];
}

#include "_window.cpp"

struct vdb_globals_t
{
    const char *active_label;
    bool initialized;
    int note_index;
    bool is_first_frame;
    bool taa_begun;
    bool tss_begun;
    render_texture_t *current_render_texture;
};

static vdb_globals_t vdb = {0};

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

namespace uistuff
{
    static bool escape_eaten;
    static bool sketch_mode_active;
    static bool ruler_mode_active;

    static void ExitDialog();
    static void WindowSizeDialog();
    static void FramegrabDialog();
}

bool vdbIsFirstFrame() { return vdb.is_first_frame; }

void vdbDetachGLContext() { window::DetachGLContext(); }

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
        settings::LoadOrDefault(VDB_SETTINGS_FILENAME);
        window::Open(settings::window_x, settings::window_y, settings::window_w, settings::window_h);
        ImGui::CreateContext();
        ImGui_ImplSdlGL3_Init(window::sdl_window);
        ImGui::StyleColorsDark();
        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF((const char*)source_sans_pro_compressed_data, source_sans_pro_compressed_size, VDB_FONT_HEIGHT);
        ImGui::GetStyle().WindowBorderSize = 0.0f;
        vdb.initialized = true;

        // Assuming user uploads images that are one-byte packed?
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        CheckGLError();
    }

    window::EnsureGLContextIsCurrent();
    window::PollEvents();

    static int save_settings_counter = 60;
    if (save_settings_counter >= 0)
        save_settings_counter--;
    if (settings::window_x != window::window_x ||
        settings::window_y != window::window_y ||
        settings::window_w != window::window_width ||
        settings::window_h != window::window_height)
    {
        settings::window_x = window::window_x;
        settings::window_y = window::window_y;
        settings::window_w = window::window_width;
        settings::window_h = window::window_height;
        // if the settings changed and we haven't saved in a while (to
        // spare disk usage) then we save to disk.
        if (save_settings_counter < 0)
        {
            save_settings_counter = 60;
            settings::Save(VDB_SETTINGS_FILENAME);
        }
    }

    if (keys::pressed[SDL_SCANCODE_F10]) { settings::Save(VDB_SETTINGS_FILENAME); is_first_frame = true; return false; }
    if (keys::pressed[SDL_SCANCODE_F5]) { settings::Save(VDB_SETTINGS_FILENAME); is_first_frame = true; skip_label = label; return false; }
    if (window::should_quit) { settings::Save(VDB_SETTINGS_FILENAME); window::Close(); exit(0); }

    transform::Reset();
    vdbResetMouseOverState();
    uistuff::escape_eaten = false;
    vdb.note_index = 0;
    transform::viewport_left = 0;
    transform::viewport_bottom = 0;
    transform::viewport_width = window::framebuffer_width;
    transform::viewport_height = window::framebuffer_height;
    mouse::ndc = vdbWindowToNDC((float)mouse::x, (float)mouse::y);

    ImGui_ImplSdlGL3_NewFrame(window::sdl_window);
    CheckGLError();

    if (VDB_HOTKEY_SKETCH_MODE) uistuff::sketch_mode_active = !uistuff::sketch_mode_active;

    if (uistuff::sketch_mode_active)
    {
        if (keys::pressed[SDL_SCANCODE_ESCAPE])
        {
            uistuff::sketch_mode_active = false;
            uistuff::escape_eaten = true;
        }
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

    if (VDB_HOTKEY_RULER_MODE) uistuff::ruler_mode_active = !uistuff::ruler_mode_active;

    if (uistuff::ruler_mode_active)
    {
        if (keys::pressed[SDL_SCANCODE_ESCAPE])
        {
            uistuff::ruler_mode_active = false;
            uistuff::escape_eaten = true;
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
    glViewport(0, 0, window::framebuffer_width, window::framebuffer_height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    CheckGLError();

    return true;
}

void vdbEndFrame()
{
    if (vdb.taa_begun) vdbEndTAA();
    if (vdb.tss_begun) vdbEndTSS();

    if (uistuff::sketch_mode_active)
        vdbSketchModePresent();

    if (uistuff::ruler_mode_active)
        vdbRulerModePresent();

    if (framegrab::active)
    {
        framegrab_options_t opt = framegrab::options;

        if (opt.draw_imgui)
        {
            ImGui::Render();
            ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
        }

        GLenum format = opt.alpha_channel ? GL_RGBA : GL_RGB;
        int channels = opt.alpha_channel ? 4 : 3;
        int width = window::framebuffer_width;
        int height = window::framebuffer_height;
        unsigned char *data = (unsigned char*)malloc(width*height*channels);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

        if (!opt.draw_imgui)
        {
            ImGui::Render();
            ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
        }

        framegrab::SaveFrame(data, width, height, channels, format);

        free(data);

        if (keys::pressed[SDL_SCANCODE_ESCAPE])
        {
            framegrab::StopRecording();
            uistuff::escape_eaten = true;
        }
        else if (VDB_HOTKEY_FRAMEGRAB)
        {
            framegrab::StopRecording();
        }
    }
    else
    {
        uistuff::WindowSizeDialog();
        uistuff::FramegrabDialog();
        uistuff::ExitDialog();
        ImGui::Render();
        ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
    }

    window::SwapBuffers(1.0f/60.0f);
    CheckGLError();
}

static void uistuff::ExitDialog()
{
    bool escape = keys::pressed[SDL_SCANCODE_ESCAPE];
    if (escape && !uistuff::escape_eaten && settings::never_ask_on_exit)
    {
        window::should_quit = true;
        return;
    }
    if (escape && !uistuff::escape_eaten && !settings::never_ask_on_exit)
    {
        ImGui::OpenPopup("Do you want to exit?##popup_exit");
    }
    if (ImGui::BeginPopupModal("Do you want to exit?##popup_exit", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::Button("Yes"))
        {
            window::should_quit = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Never ask me again", &settings::never_ask_on_exit);
        if (escape && !ImGui::IsWindowAppearing())
        {
            uistuff::escape_eaten = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void uistuff::WindowSizeDialog()
{
    if (VDB_HOTKEY_WINDOW_SIZE)
    {
        ImGui::OpenPopup("Set window size##popup");
        ImGui::CaptureKeyboardFromApp(true);
    }
    if (ImGui::BeginPopupModal("Set window size##popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static int width = 0, height = 0;
        if (ImGui::IsWindowAppearing())
        {
            width = window::window_width;
            height = window::window_height;
        }

        static bool topmost = false;
        ImGui::InputInt("Width", &width);
        ImGui::InputInt("Height", &height);
        ImGui::Separator();
        ImGui::Checkbox("Topmost", &topmost);

        if (ImGui::Button("OK", ImVec2(120,0)) || keys::pressed[SDL_SCANCODE_RETURN])
        {
            window::SetSize(width, height, topmost);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120,0)))
        {
            ImGui::CloseCurrentPopup();
        }
        if (keys::pressed[SDL_SCANCODE_ESCAPE])
        {
            ImGui::CloseCurrentPopup();
            uistuff::escape_eaten = true;
        }
        ImGui::EndPopup();
    }
}

static void uistuff::FramegrabDialog()
{
    using namespace ImGui;
    bool enter_button = keys::pressed[SDL_SCANCODE_RETURN];
    bool escape_button = keys::pressed[SDL_SCANCODE_ESCAPE];
    if (VDB_HOTKEY_FRAMEGRAB)
    {
        OpenPopup("Take screenshot##popup");
        CaptureKeyboardFromApp(true);
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
                framegrab_options_t opt = {0};
                opt.filename = filename;
                opt.reset_counter = !do_continue;
                opt.start_from = start_from;
                opt.draw_imgui = draw_imgui;
                opt.draw_cursor = draw_cursor;
                opt.alpha_channel = alpha;
                framegrab::TakeScreenshot(opt);
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
                framegrab_options_t opt = {0};
                opt.filename = filename;
                opt.alpha_channel = alpha;
                opt.draw_cursor = draw_cursor;
                opt.draw_imgui = draw_imgui;
                opt.video_frame_cap = frame_cap;
                opt.reset_counter = !do_continue;
                framegrab::RecordImageSequence(opt);
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
                framegrab_options_t opt = {0};
                opt.filename = filename;
                opt.alpha_channel = alpha;
                opt.draw_cursor = draw_cursor;
                opt.draw_imgui = draw_imgui;
                opt.ffmpeg_crf = crf;
                opt.ffmpeg_fps = framerate;
                opt.video_frame_cap = frame_cap;
                framegrab::RecordFFmpeg(opt);
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
            uistuff::escape_eaten = true;
        }
        EndPopup();
    }
}
