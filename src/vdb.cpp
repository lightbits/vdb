#include "vdb_config.h"

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
#if VDB_IMGUI_FREETYPE==1
#include "_imgui_freetype.cpp"
#endif
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
#include "_gl_error.cpp"
#include "_settings.cpp"
#include "_framegrab.cpp"
#include "data/open_sans_regular.cpp"
// #include "data/source_sans_pro.cpp"
#include "_shader.cpp"
#include "_sketch_mode.cpp"
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
#include "_uistuff.cpp"
#include "vdb_var.cpp"

namespace vdb
{
    static bool initialized;
    static bool is_first_frame;
    static bool is_different_label;
    static int loaded_font_size;
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

float vdbGetRenderScale()
{
    if (vdb::frame_settings->render_scale_down > 0)
        return 1.0f / (1 << vdb::frame_settings->render_scale_down);
    else
        return 1.0f;
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
        ImGui::GetStyle().WindowBorderSize = 0.0f;
    }

    if (vdb::loaded_font_size != settings.font_size)
    {
        vdb::loaded_font_size = settings.font_size;
        ImGui_ImplSdlGL3_InvalidateDeviceObjects();
        ImGui::GetIO().Fonts->Clear();
        const char *data = (const char*)open_sans_regular_compressed_data;
        const unsigned int size = open_sans_regular_compressed_size;
        float height = (float)settings.font_size;
        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(data, size, height);
        #if VDB_IMGUI_FREETYPE==1
        imgui_freetype::BuildFontAtlas();
        #endif
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

    // Note: uistuff is checked at BeginFrame instead of EndFrame because we want it to have
    // priority over ImGui panels created by the user.
    uistuff::escape_eaten = false;
    uistuff::SketchNewFrame();
    uistuff::RulerNewFrame();

    glDepthMask(GL_TRUE);
    glClearDepth(1.0f);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);

    if (vdb::frame_settings->render_scale_down > 0)
    {
        int n = vdb::frame_settings->render_scale_down;
        int w = window::framebuffer_width >> n;
        int h = window::framebuffer_height >> n;
        upsample_filter::Begin(w, h, vdb::frame_settings->render_scale_up);
        glDepthMask(GL_TRUE);
        glClearDepth(1.0f);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glDepthMask(GL_FALSE);
    }

    if (vdb::frame_settings->render_scale_up > 0)
    {
        int n_down = vdb::frame_settings->render_scale_down;
        int n_up = vdb::frame_settings->render_scale_up;
        int w = window::framebuffer_width >> n_down;
        int h = window::framebuffer_height >> n_down;
        vdbVec2 d;
        upsample_filter::GetSubpixelOffset(w, h, n_up, &d.x, &d.y);
        SetImmediateOffsetNDC(d);
    }
    else
    {
        SetImmediateOffsetNDC(vdbVec2(0.0f, 0.0f));
    }

    if (vdb::frame_settings->camera_type != VDB_CAMERA_DISABLED)
    {
        frame_settings_t *fs = vdb::frame_settings;
        if      (fs->camera_type == VDB_CAMERA_TRACKBALL) vdbCameraTrackball(fs->init_radius);
        else if (fs->camera_type == VDB_CAMERA_TURNTABLE) vdbCameraTurntable(fs->init_radius, fs->init_look_at);
        else                                              vdbCamera2D(fs->init_radius);

        if (fs->camera_type != VDB_CAMERA_PLANAR)
        {
            vdbDepthTest(true);
            vdbDepthWrite(true);
            vdbClearDepth(1.0f);
            vdbPerspective(fs->y_fov, fs->min_depth, fs->max_depth);

            // We do PushMatrix to save current state for drawing grid in vdbEndFrame

            // pre-permutation transform
            vdbPushMatrix();
            switch (fs->camera_floor)
            {
                case VDB_FLOOR_XY: vdbMultMatrix(vdbInitMat4(1,0,0,0, 0,0,1,0, 0,-1,0,0, 0,0,0,1).data); break;
                case VDB_FLOOR_YZ: vdbMultMatrix(vdbInitMat4(0,0,-1,0, 1,0,0,0, 0,-1,0,0, 0,0,0,1).data); break;
            }

            // pre-scaling transform
            vdbPushMatrix();
            vdbMultMatrix(vdbMatScale(1.0f/fs->grid_scale, 1.0f/fs->grid_scale, 1.0f/fs->grid_scale).data);
        }
    }

    CheckGLError();

    return true;
}

void vdbEndFrame()
{
    ResetImmediateGLState();

    if (vdb::frame_settings->render_scale_down > 0)
    {
        SetImmediateOffsetNDC(vdbVec2(0.0f, 0.0f));
        upsample_filter::End();
    }

    if (vdb::frame_settings->camera_type != VDB_CAMERA_DISABLED &&
        vdb::frame_settings->camera_type != VDB_CAMERA_PLANAR)
    {
        frame_settings_t *fs = vdb::frame_settings;
        vdbDepthTest(true);
        vdbPopMatrix(); // pre-scaling

        // draw colored XYZ axes
        if (fs->grid_visible)
        {
            float t = 10.0f;
            vdbLineWidth(1.0f);
            vdbBeginLines();
            if (fs->camera_floor != VDB_FLOOR_YZ) // hide x
            {
                vdbColor(1.0f, 0.2f, 0.32f, 0.0f); vdbVertex(-t, 0.0f, 0.0f);
                vdbColor(1.0f, 0.2f, 0.32f, 0.2f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(1.0f, 0.2f, 0.32f, 0.5f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(1.0f, 0.2f, 0.32f, 0.0f); vdbVertex(+t, 0.0f, 0.0f);
            }

            if (fs->camera_floor != VDB_FLOOR_XZ) // hide y
            {
                vdbColor(0.54f, 0.85f, 0.0f, 0.0f); vdbVertex(0.0f, -t, 0.0f);
                vdbColor(0.54f, 0.85f, 0.0f, 0.2f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(0.54f, 0.85f, 0.0f, 0.5f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(0.54f, 0.85f, 0.0f, 0.0f); vdbVertex(0.0f, +t, 0.0f);
            }

            if (fs->camera_floor != VDB_FLOOR_XY) // hide z
            {
                vdbColor(0.16f, 0.56f, 0.99f, 0.0f); vdbVertex(0.0f, 0.0f, -t);
                vdbColor(0.16f, 0.56f, 0.99f, 0.2f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(0.16f, 0.56f, 0.99f, 0.5f); vdbVertex(0.0f, 0.0f, 0.0f);
                vdbColor(0.16f, 0.56f, 0.99f, 0.0f); vdbVertex(0.0f, 0.0f, +t);
            }
            vdbEnd();
        }

        vdbPopMatrix(); // pre-permutation

        if (fs->cube_visible)
        {
            vdbLineWidth(1.0f);
            vdbColor(1.0f, 1.0f, 1.0f, 0.3f);
            vdbLineCube(1.0f, 1.0f, 1.0f);
        }

        // draw major and minor grid lines
        if (fs->grid_visible)
        {
            vdbLineWidth(1.0f);
            // todo: this should be a draw-list
            vdbPushMatrix();
            vdbMultMatrix(vdbMatScale(10.0f, 10.0f, 10.0f).data);
            vdbBeginLines();
            for (int major = -9; major <= +9; major++)
            {
                for (int minor = 1; minor <= 9; minor++)
                {
                    float t = major/10.0f + minor/100.0f;
                    float alpha = 0.25f*(1.0f - fabsf(t));
                    alpha *= alpha;
                    vdbColor(1,1,1, 0.00f); vdbVertex(t, 0.0f, -1.0f);
                    vdbColor(1,1,1, alpha); vdbVertex(t, 0.0f, 0.0f);
                    vdbColor(1,1,1, alpha); vdbVertex(t, 0.0f, 0.0f);
                    vdbColor(1,1,1, 0.00f); vdbVertex(t, 0.0f, +1.0f);

                    vdbColor(1,1,1, 0.00f); vdbVertex(-1.0f, 0.0f, t);
                    vdbColor(1,1,1, alpha); vdbVertex(0.0f, 0.0f, t);
                    vdbColor(1,1,1, alpha); vdbVertex(0.0f, 0.0f, t);
                    vdbColor(1,1,1, 0.00f); vdbVertex(+1.0f, 0.0f, t);
                }

                if (major == 0)
                    continue;
                float t = major/10.0f;
                float alpha = 0.40f*(1.0f - fabsf(t));
                alpha *= alpha;
                vdbColor(1,1,1, 0.00f); vdbVertex(t, 0.0f, -1.0f);
                vdbColor(1,1,1, alpha); vdbVertex(t, 0.0f, 0.0f);
                vdbColor(1,1,1, alpha); vdbVertex(t, 0.0f, 0.0f);
                vdbColor(1,1,1, 0.00f); vdbVertex(t, 0.0f, +1.0f);

                vdbColor(1,1,1, 0.00f); vdbVertex(-1.0f, 0.0f, t);
                vdbColor(1,1,1, alpha); vdbVertex(0.0f, 0.0f, t);
                vdbColor(1,1,1, alpha); vdbVertex(0.0f, 0.0f, t);
                vdbColor(1,1,1, 0.00f); vdbVertex(+1.0f, 0.0f, t);
            }
            vdbEnd();
            vdbPopMatrix();
        }
        vdbDepthTest(false);
    }

    quick_var::EndFrame();

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
        uistuff::MainMenuBar(vdb::frame_settings);
        uistuff::WindowSizeDialog();
        uistuff::FramegrabDialog();
        uistuff::ExitDialog();
        uistuff::RulerEndFrame();
        uistuff::SketchEndFrame();
        ImGui::Render();
        ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
    }

    window::SwapBuffers(1.0f/60.0f);
    CheckGLError();
}
