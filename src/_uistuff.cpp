namespace uistuff
{
    static bool escape_eaten;
    static bool sketch_mode_active;
    static bool ruler_mode_active;
    static bool window_size_dialog_should_open;
    static bool take_screenshot_should_open;
    static bool record_video_should_open;

    // note: this is in window coordinates (ImGui coordinates)
    // note: this may change from frame to frame!
    static float main_menu_bar_height = 0.0f;

    namespace ruler
    {
        // window coordinates (ImGui coordinates)
        static vdbVec2 mouse;
        static vdbVec2 a,b;
    }

    static void MainMenuBar(frame_settings_t *fs);
    static void ExitDialog();
    static void WindowSizeDialog();
    static void FramegrabDialog();
    static void RulerNewFrame();
    static void RulerEndFrame();
    static void SketchNewFrame();
    static void SketchEndFrame();
}

static void uistuff::MainMenuBar(frame_settings_t *fs)
{
    using namespace uistuff;

    // hide/show menu
    if (VDB_HOTKEY_TOGGLE_MENU)
        settings.show_main_menu = !settings.show_main_menu;
    if (!settings.show_main_menu)
    {
        main_menu_bar_height = 0.0f;
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
    ImGui::BeginMainMenuBar();
    main_menu_bar_height = ImGui::GetWindowHeight();
    if (ImGui::BeginMenu("Camera"))
    {
        #define ITEM(s, e) if (ImGui::MenuItem(s, NULL, fs->camera_type == e)) fs->camera_type = e;
        ITEM("Disabled", VDB_CAMERA_DISABLED);
        // ImGui::SameLine(); ShowHelpMarker("The built-in camera is disabled. All projection and matrix transforms are controlled through your API calls.");
        ITEM("Planar", VDB_CAMERA_PLANAR);
        // ImGui::SameLine(); ShowHelpMarker("A 2D camera (left: pan, right: rotate, wheel: zoom).");
        ITEM("Trackball", VDB_CAMERA_TRACKBALL);
        // ImGui::SameLine(); ShowHelpMarker("A 3D camera (left: rotate, WASD: move, wheel: zoom).");
        ITEM("Turntable", VDB_CAMERA_TURNTABLE);
        // ImGui::SameLine(); ShowHelpMarker("A 3D camera (left: rotate, wheel: zoom).");
        #undef ITEM
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Grid"))
    {
        ImGui::MenuItem("Show grid", NULL, &fs->grid_visible);
        ImGui::MenuItem("Show cube", NULL, &fs->cube_visible);
        ImGui::SameLine(); ShowHelpMarker("Draw a unit cube (from -0.5 to +0.5 in each axis).");
        ImGui::PushItemWidth(60.0f);
        ImGui::DragFloat("Major div.", &fs->grid_scale);
        ImGui::PopItemWidth();
        ImGui::SameLine(); ShowHelpMarker("The length (in your units) between the major grid lines (the brighter ones).");
        ImGui::RadioButton("XY", &fs->camera_floor, VDB_FLOOR_XY); ImGui::SameLine();
        ImGui::RadioButton("XZ", &fs->camera_floor, VDB_FLOOR_XZ); ImGui::SameLine();
        ImGui::RadioButton("YZ", &fs->camera_floor, VDB_FLOOR_YZ);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Settings"))
    {
        ImGui::MenuItem("Show menu", "Alt+M", &settings.show_main_menu);
        ImGui::MenuItem("Window size", "Alt+W", &window_size_dialog_should_open);
        ImGui::MenuItem("Never ask on exit", NULL, &settings.never_ask_on_exit);
        if (ImGui::BeginMenu("Font"))
        {
            if (ImGui::MenuItem("Larger")) settings.font_size += 1;
            if (ImGui::MenuItem("Smaller") && settings.font_size > 1) settings.font_size -= 1;
            ImGui::Separator();
            if (ImGui::MenuItem("Reset")) settings.font_size = (int)VDB_DEFAULT_FONT_SIZE;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Render scale"))
        {
            #define ITEM(label, down, up) \
                if (ImGui::MenuItem(label, NULL, fs->render_scale_down==down && fs->render_scale_up==up)) { \
                    fs->render_scale_down = down; \
                    fs->render_scale_up = up; \
                }
            ITEM("1/1", 0, 0);
            ITEM("1/2", 1, 0);
            ITEM("1/4", 2, 0);
            ITEM("1/8", 3, 0);
            #if 0
            ImGui::Separator();
            ITEM("2/2", 1, 1);
            ITEM("2/4", 2, 1);
            ITEM("2/8", 3, 1);
            ImGui::Separator();
            ITEM("4/4", 2, 2);
            ITEM("4/8", 3, 2);
            ImGui::Separator();
            ITEM("8/8", 3, 3);
            #endif
            #undef ITEM
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Tools"))
    {
        if (ImGui::MenuItem("Take screenshot", "Alt+S")) take_screenshot_should_open = true;
        if (ImGui::MenuItem("Record video", "Alt+S")) record_video_should_open = true;
        ImGui::MenuItem("Ruler", "Alt+R", &ruler_mode_active);
        ImGui::MenuItem("Draw", "Alt+D", &sketch_mode_active);
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
    ImGui::PopStyleVar();
}

static void uistuff::ExitDialog()
{
    bool escape = keys::pressed[SDL_SCANCODE_ESCAPE];
    if (escape && !uistuff::escape_eaten && settings.never_ask_on_exit)
    {
        window::should_quit = true;
        return;
    }
    if (escape && !uistuff::escape_eaten && !settings.never_ask_on_exit)
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
        ImGui::Checkbox("Never ask me again", &settings.never_ask_on_exit);
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
    if (window_size_dialog_should_open || (VDB_HOTKEY_WINDOW_SIZE))
    {
        window_size_dialog_should_open = false;
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
    if (take_screenshot_should_open ||
        record_video_should_open ||
        (VDB_HOTKEY_FRAMEGRAB))
    {
        take_screenshot_should_open = false;
        record_video_should_open = false;
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
            if (Button("OK [Enter]", ImVec2(120,0)) || enter_button)
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

            if (Button("Start [Enter]", ImVec2(120,0)) || enter_button)
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

            if (Button("Start [Enter]", ImVec2(120,0)) || enter_button)
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

static void uistuff::RulerNewFrame()
{
    if (VDB_HOTKEY_RULER_MODE)
        ruler_mode_active = !ruler_mode_active;

    if (ruler_mode_active)
    {
        if (keys::pressed[SDL_SCANCODE_ESCAPE])
        {
            ruler_mode_active = false;
            escape_eaten = true;
        }

        static bool dragging = false;
        ruler::mouse = vdbGetMousePos();
        if (vdbIsMouseLeftDown())
        {
            if (!dragging)
            {
                dragging = true;
                ruler::a = ruler::mouse;
                ruler::b = ruler::mouse;
            }
            else
            {
                ruler::b = ruler::mouse;
            }
        }
        else
        {
            dragging = false;
        }

        // force all subsequent calls to vdb{Is,Was}{Mouse,Key}{UpDownPressed} to return false
        ImGui::GetIO().WantCaptureKeyboard = true;
        ImGui::GetIO().WantCaptureMouse = true;
    }
}

static void uistuff::RulerEndFrame()
{
    if (!ruler_mode_active)
        return;

    using namespace ruler;
    vdbVec2 ndc_a = vdbWindowToNDC(a.x, a.y);
    vdbVec3 model_a = vdbNDCToModel(ndc_a.x, ndc_a.y);
    vdbVec2 ndc_b = vdbWindowToNDC(b.x, b.y);
    vdbVec3 model_b = vdbNDCToModel(ndc_b.x, ndc_b.y);
    float dx = model_b.x - model_a.x;
    float dy = model_b.y - model_a.y;
    float distance = sqrtf(dx*dx + dy*dy);
    float distance_px = sqrtf((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));

    ImDrawList *draw = ImGui::GetOverlayDrawList();

    ImU32 fg = IM_COL32(255,255,255,255);
    ImU32 bg = IM_COL32(0,0,0,255);

    if (distance_px > 1.0f)
    {
        float thickness = 2.0f;
        draw->AddLine(ImVec2(a.x,a.y), ImVec2(b.x,b.y), bg, thickness+2.0f);
        draw->AddCircleFilled(ImVec2(a.x,a.y), 5.0f, bg);
        draw->AddCircleFilled(ImVec2(b.x,b.y), 5.0f, bg);
        draw->AddLine(ImVec2(a.x,a.y), ImVec2(b.x,b.y), fg, thickness);
        draw->AddCircleFilled(ImVec2(a.x,a.y), 4.0f, fg);
        draw->AddCircleFilled(ImVec2(b.x,b.y), 4.0f, fg);
    }

    ImGui::BeginMainMenuBar();
    ImGui::Separator();
    ImGui::Text("%4d, %4d px", (int)mouse.x, (int)mouse.y);
    ImGui::Separator();
    ImGui::Text("%4d px", (int)distance_px);
    ImGui::Separator();
    ImGui::Text("%g user", distance);
    ImGui::EndMainMenuBar();
}

static void uistuff::SketchNewFrame()
{
    if (VDB_HOTKEY_SKETCH_MODE)
        sketch_mode_active = !sketch_mode_active;

    if (sketch_mode_active)
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
        float x = vdbGetMousePos().x;
        float y = vdbGetMousePos().y;
        sketch_mode::Update(undo, redo, clear, click, x, y);

        // force all subsequent calls to vdb{Is,Was}{Mouse,Key}{UpDownPressed} to return false
        ImGui::GetIO().WantCaptureKeyboard = true;
        ImGui::GetIO().WantCaptureMouse = true;
    }
}

static void uistuff::SketchEndFrame()
{
    if (!uistuff::sketch_mode_active)
        return;

    static float width = 2.0f;
    static float brightness = 0.0f;

    // draw sketching tool bar
    ImGui::BeginMainMenuBar();
    float h_menu = ImGui::GetFrameHeight();
    float h_rect = ImGui::GetTextLineHeight();
    {
        ImGui::Separator();
        ImGui::PushID("sketch colors");
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f,-1.0f));
        ImDrawList *draw = ImGui::GetWindowDrawList();
        for (int i = 0; i < sketch_mode::num_colors; i++)
        {
            ImGui::PushID(i);
            ImGui::InvisibleButton("##", ImVec2(16.0f, h_rect));
            ImVec2 a = ImGui::GetItemRectMin();
            ImVec2 b = ImGui::GetItemRectMax();
            a.y = 0.5f*(h_menu - h_rect);
            b.y = 0.5f*(h_menu + h_rect);
            ImU32 color = sketch_mode::palette[i];
            draw->AddRectFilled(a, b, color);
            if (ImGui::IsItemClicked())
                sketch_mode::s.color_index = i;
            ImGui::PopID();
        }
        ImGui::PopStyleVar();
        ImGui::PushItemWidth(64.0f);
        ImGui::SliderFloat("brightness", &brightness, -1.0f, +1.0f);
        ImGui::PopItemWidth();
        ImGui::PopID();
    }
    ImGui::EndMainMenuBar();

    int num_lines = sketch_mode::s.num_lines;
    sketch_mode_line_t *lines = sketch_mode::s.lines;
    bool is_drawing = sketch_mode::s.is_drawing;

    ImDrawList *user_draw_list = ImGui::GetOverlayDrawList();

    // draw brightness overlay
    {
        int alpha = (int)(fabsf(brightness)*255);
        ImU32 color = brightness > 0.0f ? IM_COL32(255,255,255,alpha) : IM_COL32(0,0,0,alpha);
        ImVec2 a = ImVec2(0.0f, h_menu);
        ImVec2 b = ImGui::GetIO().DisplaySize;
        user_draw_list->AddRectFilled(a, b, color);
    }

    // draw lines
    {
        int n = (is_drawing) ? num_lines+1 : num_lines;
        ImVec2 next_a;
        for (int i = 0; i < n; i++)
        {
            ImVec2 a;
            if (lines[i].connected)
            {
                assert(i > 0);
                a = next_a;
            }
            else
            {
                a.x = lines[i].x1;
                a.y = lines[i].y1;
            }
            ImVec2 b = ImVec2(lines[i].x2, lines[i].y2);
            if (i < n-1 && lines[i+1].connected)
                next_a = b;
            user_draw_list->AddLine(a, b, lines[i].color, width);
        }
    }
}
