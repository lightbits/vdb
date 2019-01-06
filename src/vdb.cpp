#include "SDL.h"
#ifdef VDB_DEBUG
#include "glad/glad_3_1_debug.c"
#else
#include "glad/glad_3_1_release.c"
#endif
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

typedef void (APIENTRYP GLVERTEXATTRIBDIVISORPROC)(GLuint, GLuint);
GLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "vdb.h"
#include "vdb_config.h"
#include "_gl_error.cpp"
#include "_settings.cpp"
#include "_framegrab.cpp"
#include "data/open_sans_regular.cpp"
// #include "data/source_sans_pro.cpp"
#include "_shader.cpp"
#include "_sketch_mode.cpp"
#include "_ruler_mode.cpp"
#include "_matrix.cpp"
#include "_matrix_stack.cpp"
#include "vdb_keyboard.cpp"
#include "vdb_mouse.cpp"
#include "_window.cpp"
#include "vdb_image.cpp"
#include "vdb_render_texture.cpp"
#include "vdb_transform.cpp"
#include "vdb_camera.cpp"
#include "vdb_immediate.cpp"
#include "vdb_immediate_util.cpp"
#include "vdb_shader.cpp"
#include "vdb_filter.cpp"
#include "vdb_var.cpp"
#include "_uistuff.cpp"

namespace vdb
{
    static bool initialized;
    static bool is_first_frame;
    static bool is_different_label;
    static frame_settings_t *frame_settings;
}

bool vdbIsFirstFrame()
{
    return vdb::is_first_frame;
}

bool vdbIsDifferentLabel()
{
    return vdb::is_different_label;
}

void vdbDetachGLContext()
{
    window::DetachGLContext();
}

bool vdbBeginFrame(const char *label)
{
    static const char *skip_label = NULL;
    static const char *prev_label = NULL;
    static bool is_first_frame = true;
    vdb::is_first_frame = is_first_frame;
    vdb::is_different_label = label != prev_label;
    prev_label = label; // todo: strdup
    if (skip_label == label)
        return false;
    is_first_frame = false; // todo: first frame detection is janky.
                            // consider e.g. a for loop with single-stepping

    if (!vdb::initialized)
    {
        vdb::initialized = true;

        settings.LoadOrDefault(VDB_SETTINGS_FILENAME);
        window_settings_t ws = settings.window;
        window::Open(ws.x, ws.y, ws.width, ws.height);
        CheckGLError();

        ImGui::CreateContext();
        ImGui_ImplSdlGL3_Init(window::sdl_window);
        ImGui::StyleColorsDark();
        // ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF((const char*)source_sans_pro_compressed_data, source_sans_pro_compressed_size, VDB_FONT_HEIGHT);
        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF((const char*)open_sans_regular_compressed_data, open_sans_regular_compressed_size, VDB_FONT_HEIGHT);
        ImGui::GetStyle().WindowBorderSize = 0.0f;
    }

    if (vdbIsFirstFrame())
    {
        // see if we loaded a frame settings matching this frame's label
        frame_settings_t *fs = NULL;
        for (int i = 0; i < settings.num_frames; i++)
        {
            if (strcmp(settings.frames[i].name, label) == 0)
            {
                fs = settings.frames + i;
                break;
            }
        }

        // otherwise add a new one (but only if there's space)
        if (!fs)
        {
            if (settings.num_frames < MAX_FRAME_SETTINGS)
                fs = settings.frames + (settings.num_frames++);
            else
                fs = new frame_settings_t; // will not be saved
            fs->name = strdup(label);
            DefaultFrameSettings(fs);
        }

        assert(fs);

        vdb::frame_settings = fs;
    }

    window::EnsureGLContextIsCurrent();
    window::PollEvents();

    static int save_settings_counter = VDB_SAVE_SETTINGS_PERIOD;
    save_settings_counter--;
    if (save_settings_counter <= 0)
    {
        save_settings_counter += VDB_SAVE_SETTINGS_PERIOD;
        settings.Save(VDB_SETTINGS_FILENAME);
    }

    if (keys::pressed[SDL_SCANCODE_F10])
    {
        settings.Save(VDB_SETTINGS_FILENAME);
        is_first_frame = true;
        return false;
    }
    if (keys::pressed[SDL_SCANCODE_F5])
    {
        settings.Save(VDB_SETTINGS_FILENAME);
        is_first_frame = true;
        skip_label = label;
        return false;
    }
    if (window::should_quit)
    {
        settings.Save(VDB_SETTINGS_FILENAME);
        window::Close();
        exit(0);
    }

    transform::NewFrame();
    mouse_over::NewFrame();
    mouse::NewFrame();
    immediate_util::NewFrame();
    immediate::NewFrame();
    ImGui_ImplSdlGL3_NewFrame(window::sdl_window);
    quick_var::NewFrame();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Assuming user uploads images that are one-byte packed
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Note: uistuff is checked at BeginFrame instead of EndFrame because we want it to have
    // priority over ImGui panels created by the user.
    uistuff::escape_eaten = false;

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

    CheckGLError();

    #if 1
    if (vdb::frame_settings->camera_type != VDB_CAMERA_USER)
    {
        frame_settings_t *fs = vdb::frame_settings;
        if (fs->camera_type == VDB_CAMERA_TRACKBALL)
        {
            vdbCameraTrackball(fs->init_radius);
            vdbDepthTest(true);
            vdbDepthWrite(true);
            vdbClearDepth(1.0f);
            vdbPerspective(fs->y_fov, fs->min_depth, fs->max_depth);
        }
        else if (fs->camera_type == VDB_CAMERA_TURNTABLE)
        {
            vdbCameraTurntable(fs->init_radius, fs->init_look_at);
            vdbDepthTest(true);
            vdbDepthWrite(true);
            vdbClearDepth(1.0f);
            vdbPerspective(fs->y_fov, fs->min_depth, fs->max_depth);
        }
        else
        {
            vdbCamera2D(fs->init_radius);
        }

        if (fs->camera_type != VDB_CAMERA_2D)
        {
            // apply camera pre-transformation
            vdbPushMatrix(); // save current state for later (grid drawing)
            switch (fs->camera_floor)
            {
                case VDB_FLOOR_XY: vdbMultMatrix(vdbInitMat4(1,0,0,0, 0,0,1,0, 0,-1,0,0, 0,0,0,1).data); break;
                case VDB_FLOOR_YZ: vdbMultMatrix(vdbInitMat4(0,0,-1,0, 1,0,0,0, 0,-1,0,0, 0,0,0,1).data); break;
            }
        }
    }
    #endif

    return true;
}

void vdbEndFrame()
{
    ResetImmediateGLState();

    if (filter::taa_begun) vdbEndTAA();
    if (filter::tss_begun) vdbEndTSS();

    if (vdb::frame_settings->camera_type != VDB_CAMERA_USER &&
        vdb::frame_settings->camera_type != VDB_CAMERA_2D)
    {
        frame_settings_t *fs = vdb::frame_settings;
        float scale = fs->grid_scale;

        vdbDepthTest(true);

        if (fs->cube_visible)
        {
            vdbLineWidth(1.0f);
            vdbColor(1.0f, 1.0f, 1.0f, 0.3f);
            vdbLineCube(fs->cube_scale, fs->cube_scale, fs->cube_scale);
        }

        // draw colored XYZ axes (only the two axes of the grid plane though)
        if (fs->grid_visible)
        {
            vdbLineWidth(1.0f);
            vdbBeginLines();
            if (fs->camera_floor != VDB_FLOOR_YZ) // hide x
            {
                vdbColor(1.0f, 0.2f, 0.32f, 0.0f); vdbVertex(-scale, 0.0f, 0.0f);
                vdbColor(1.0f, 0.2f, 0.32f, 0.4f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(1.0f, 0.2f, 0.32f, 0.4f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(1.0f, 0.2f, 0.32f, 0.0f); vdbVertex(+scale, 0.0f, 0.0f);
            }

            if (fs->camera_floor != VDB_FLOOR_XZ) // hide y
            {
                vdbColor(0.54f, 0.85f, 0.0f, 0.0f); vdbVertex(0.0f, -scale, 0.0f);
                vdbColor(0.54f, 0.85f, 0.0f, 0.4f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(0.54f, 0.85f, 0.0f, 0.4f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(0.54f, 0.85f, 0.0f, 0.0f); vdbVertex(0.0f, +scale, 0.0f);
            }

            if (fs->camera_floor != VDB_FLOOR_XY) // hide z
            {
                vdbColor(0.16f, 0.56f, 0.99f, 0.0f); vdbVertex(0.0f, 0.0f, -scale);
                vdbColor(0.16f, 0.56f, 0.99f, 0.4f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(0.16f, 0.56f, 0.99f, 0.4f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(0.16f, 0.56f, 0.99f, 0.0f); vdbVertex(0.0f, 0.0f, +scale);
            }
            vdbEnd();
        }

        // revert camera pre-transformation (grid is always drawn in xz camera space)
        vdbPopMatrix();

        // draw major and minor grid lines
        if (fs->grid_visible)
        {
            vdbLineWidth(1.0f);
            vdbPushMatrix();
            vdbMultMatrix(vdbMatScale(scale, scale, scale).data);
            // todo: this should be a draw-list
            vdbBeginLines();
            for (int major = -9; major <= +9; major++)
            {
                for (int minor = 1; minor <= 9; minor++)
                {
                    float t = major/10.0f + minor/100.0f;
                    float alpha = 0.25f*(1.0f - fabsf(t));
                    alpha *= alpha;
                    vdbColor(1,1,1, 0.0f);  vdbVertex(t, 0.0f, -1.0f);
                    vdbColor(1,1,1, alpha); vdbVertex(t, 0.0f, 0.0f);
                    vdbColor(1,1,1, alpha); vdbVertex(t, 0.0f, 0.0f);
                    vdbColor(1,1,1, 0.0f);  vdbVertex(t, 0.0f, +1.0f);

                    vdbColor(1,1,1, 0.0f);  vdbVertex(-1.0f, 0.0f, t);
                    vdbColor(1,1,1, alpha); vdbVertex(0.0f,   0.0f, t);
                    vdbColor(1,1,1, alpha); vdbVertex(0.0f,   0.0f, t);
                    vdbColor(1,1,1, 0.0f);  vdbVertex(+1.0f, 0.0f, t);
                }

                if (major == 0)
                    continue;
                float t = major/10.0f;
                float alpha = 0.40f*(1.0f - fabsf(t));
                alpha *= alpha;
                vdbColor(1,1,1, 0.0f);  vdbVertex(t, 0.0f, -1.0f);
                vdbColor(1,1,1, alpha); vdbVertex(t, 0.0f, 0.0f);
                vdbColor(1,1,1, alpha); vdbVertex(t, 0.0f, 0.0f);
                vdbColor(1,1,1, 0.0f);  vdbVertex(t, 0.0f, +1.0f);

                vdbColor(1,1,1, 0.0f);  vdbVertex(-1.0f, 0.0f, t);
                vdbColor(1,1,1, alpha); vdbVertex(0.0f,   0.0f, t);
                vdbColor(1,1,1, alpha); vdbVertex(0.0f,   0.0f, t);
                vdbColor(1,1,1, 0.0f);  vdbVertex(+1.0f, 0.0f, t);
            }
            vdbEnd();
            vdbPopMatrix();
        }

        vdbDepthTest(false);
    }

    quick_var::EndFrame();

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
        uistuff::CameraToolBar(vdb::frame_settings);
        ImGui::Render();
        ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
    }

    window::SwapBuffers(1.0f/60.0f);
    CheckGLError();
}
