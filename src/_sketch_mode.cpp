enum { vdb_max_sketch_lines = 1024*256 };
struct vdb_sketch_mode_t
{
    struct line_t
    {
        int color;
        bool connected;
        float x1,y1,x2,y2;
    };
    line_t lines[vdb_max_sketch_lines];
    int num_lines;
    bool is_drawing;
    int current_color;
    float brightness;
    float cursor_x,cursor_y;
};
static vdb_sketch_mode_t vdb_sketch;

static int vdb_sketch_palette[] = {
    (int)(255*1.00f), (int)(255*1.00f), (int)(255*1.00f),
    (int)(255*0.29f), (int)(255*0.50f), (int)(255*0.67f),
    (int)(255*0.71f), (int)(255*0.05f), (int)(255*0.10f),
    (int)(255*0.91f), (int)(255*0.76f), (int)(255*0.41f),
    (int)(255*0.44f), (int)(255*0.64f), (int)(255*0.40f),
    (int)(255*0.10f), (int)(255*0.10f), (int)(255*0.10f),
    (int)(255*0.62f), (int)(255*0.78f), (int)(255*0.89f),
    (int)(255*0.92f), (int)(255*0.60f), (int)(255*0.61f),
    (int)(255*1.00f), (int)(255*0.90f), (int)(255*0.64f),
    (int)(255*0.71f), (int)(255*0.84f), (int)(255*0.67f)
};

enum { vdb_sketch_num_colors = sizeof(vdb_sketch_palette)/(3*sizeof(int)) };

static ImVec2 vdb_sketch_palette_pos[] = {
    ImVec2(12.0f + 0*24.0f, 12.0f + 0*24.0f),
    ImVec2(12.0f + 1*24.0f, 12.0f + 0*24.0f),
    ImVec2(12.0f + 2*24.0f, 12.0f + 0*24.0f),
    ImVec2(12.0f + 3*24.0f, 12.0f + 0*24.0f),
    ImVec2(12.0f + 4*24.0f, 12.0f + 0*24.0f),
    ImVec2(12.0f + 0*24.0f, 12.0f + 1*24.0f),
    ImVec2(12.0f + 1*24.0f, 12.0f + 1*24.0f),
    ImVec2(12.0f + 2*24.0f, 12.0f + 1*24.0f),
    ImVec2(12.0f + 3*24.0f, 12.0f + 1*24.0f),
    ImVec2(12.0f + 4*24.0f, 12.0f + 1*24.0f)
};

void vdbSketchMode(bool undo_button,
                   bool redo_button,
                   bool clear_button,
                   bool brightness_button,
                   bool mouse_left_down,
                   float mouse_x,
                   float mouse_y)
{
    static int redo_num_lines = 0;

    bool &is_drawing = vdb_sketch.is_drawing;
    int &num_lines = vdb_sketch.num_lines;
    float &brightness = vdb_sketch.brightness;
    int &current_color = vdb_sketch.current_color;
    vdb_sketch_mode_t::line_t *lines = vdb_sketch.lines;

    vdb_sketch.cursor_x = mouse_x;
    vdb_sketch.cursor_y = mouse_y;

    if (clear_button)
    {
        num_lines = 0;
        redo_num_lines = 0;
    }
    if (undo_button)
    {
        if (num_lines > 0)
        {
            if (lines[num_lines-1].connected)
            {
                while (num_lines > 1 && lines[num_lines-1].connected)
                    num_lines--;
                num_lines--;
            }
            else
            {
                num_lines--;
            }
        }
    }
    if (redo_button)
    {
        if (num_lines < redo_num_lines)
        {
            num_lines++;
            while (num_lines < redo_num_lines && lines[num_lines].connected)
                num_lines++;
        }
    }

    static bool is_dragging = false;
    if (mouse_left_down)
    {
        int intersected_color = -1;
        for (int i = 0; i < vdb_sketch_num_colors; i++)
        {
            float dx = vdb_sketch_palette_pos[i].x - mouse_x;
            float dy = vdb_sketch_palette_pos[i].y - mouse_y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < 16.0f)
            {
                intersected_color = i;
            }
        }
        if (brightness_button)
        {
            static float start_x = 0.0f;
            static float start_y = 0.0f;
            if (!is_dragging)
            {
                start_x = mouse_x;
                start_y = mouse_y;
                is_dragging = true;
            }
            brightness = (mouse_x-start_x)/100.0f;
            if (brightness < -1.0f) brightness = -1.0f;
            if (brightness > +1.0f) brightness = +1.0f;
        }
        else if (intersected_color >= 0)
        {
            current_color = intersected_color;
        }
        else if (!is_drawing && num_lines < vdb_max_sketch_lines)
        {
            lines[num_lines].x1 = mouse_x;
            lines[num_lines].y1 = mouse_y;
            lines[num_lines].x2 = mouse_x;
            lines[num_lines].y2 = mouse_y;
            lines[num_lines].connected = false;
            lines[num_lines].color = current_color;
            is_drawing = true;
        }
        else if (num_lines < vdb_max_sketch_lines)
        {
            float new_x = mouse_x;
            float new_y = mouse_y;
            float prev_x = lines[num_lines].x2;
            float prev_y = lines[num_lines].y2;
            float begin_x = lines[num_lines].x1;
            float begin_y = lines[num_lines].y1;

            float dx1 = prev_x - begin_x;
            float dy1 = prev_y - begin_y;
            float dx2 = new_x - begin_x;
            float dy2 = new_y - begin_y;
            float dx3 = new_x - prev_x;
            float dy3 = new_y - prev_y;
            float speed = sqrtf(dx3*dx3 + dy3*dy3) / (1.0f/60.0f);
            float delta = (dx1*dx2 + dy1*dy2) / sqrtf(dx1*dx1 + dy1*dy1);
            float threshold = sqrtf(speed)/1.5f;
            if (threshold < 2.0f) threshold = 2.0f;
            if (delta > threshold || delta < -threshold)
            {
                num_lines++;
                lines[num_lines].x1 = prev_x;
                lines[num_lines].y1 = prev_y;
                lines[num_lines].x2 = new_x;
                lines[num_lines].y2 = new_y;
                lines[num_lines].connected = true;
                lines[num_lines].color = current_color;
            }
            else
            {
                lines[num_lines].x2 = new_x;
                lines[num_lines].y2 = new_y;
            }
        }
    }
    else if (is_drawing)
    {
        num_lines++;
        redo_num_lines = num_lines;
        is_drawing = false;
    }
    else if (is_dragging)
    {
        is_dragging = false;
    }
}

void vdbSketchModePresent()
{
    float thickness = 2.0f;
    bool &is_drawing = vdb_sketch.is_drawing;
    int &num_lines = vdb_sketch.num_lines;
    float &brightness = vdb_sketch.brightness;
    vdb_sketch_mode_t::line_t *lines = vdb_sketch.lines;

    static bool init = true;
    if (init)
    {
        srand(12345);
        init = false;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|
        ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoInputs|
        ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoFocusOnAppearing|
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("##UserDrawWindow", NULL, flags);
    ImDrawList *user_draw_list = ImGui::GetWindowDrawList();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    int n = (is_drawing) ? num_lines+1 : num_lines;

    // draw overlay
    {
        int alpha = (int)(fabsf(brightness)*255);
        ImU32 col = brightness > 0.0f ? IM_COL32(255,255,255,alpha) : IM_COL32(0,0,0,alpha);
        user_draw_list->AddRectFilled(ImVec2(0,0), ImGui::GetIO().DisplaySize, col);
    }

    static float noise_x1[vdb_max_sketch_lines];
    static float noise_y1[vdb_max_sketch_lines];
    static float noise_x2[vdb_max_sketch_lines];
    static float noise_y2[vdb_max_sketch_lines];
    static int frame = 0;
    frame++;
    if (frame == 8)
    {
        for (int i = 0; i < num_lines; i++)
        {
            noise_x1[i] = (-1.0f + 2.0f*(rand()%1024)/1024.0f);
            noise_y1[i] = (-1.0f + 2.0f*(rand()%1024)/1024.0f);
            noise_x2[i] = (-1.0f + 2.0f*(rand()%1024)/1024.0f);
            noise_y2[i] = (-1.0f + 2.0f*(rand()%1024)/1024.0f);
        }
        frame = 0;
    }

    ImVec2 next_a;
    for (int i = 0; i < n;i++)
    {
        ImVec2 a;
        ImVec2 b;
        if (lines[i].connected)
        {
            a = next_a;
        }
        else
        {
            a.x = lines[i].x1 + noise_x1[i];
            a.y = lines[i].y1 + noise_y1[i];
        }
        b.x = lines[i].x2 + noise_x2[i];
        b.y = lines[i].y2 + noise_y2[i];
        if (i < n-1 && lines[i+1].connected)
        {
            next_a = b;
        }
        int cr = vdb_sketch_palette[3*lines[i].color + 0];
        int cg = vdb_sketch_palette[3*lines[i].color + 1];
        int cb = vdb_sketch_palette[3*lines[i].color + 2];
        ImU32 color = IM_COL32(cr,cg,cb,255);
        #if 0
        // polyline approach (miter joins)
        if (lines[i].connected)
        {
            user_draw_list->PathLineTo(b);
        }
        else
        {
            user_draw_list->PathClear();
            user_draw_list->PathLineTo(a);
            user_draw_list->PathLineTo(b);
        }
        if (i == n-1 || !lines[i+1].connected)
        {
            user_draw_list->PathLineTo(b);
            user_draw_list->PathStroke(color, false, thickness);
        }
        #else
        // one-line-at-a-time approach
        user_draw_list->AddLine(a, b, color, thickness);
        #endif
    }

    for (int i = 0; i < vdb_sketch_num_colors; i++)
    {
        int cr = vdb_sketch_palette[3*i + 0];
        int cg = vdb_sketch_palette[3*i + 1];
        int cb = vdb_sketch_palette[3*i + 2];
        ImU32 color = IM_COL32(cr,cg,cb,255);
        ImVec2 center = vdb_sketch_palette_pos[i];
        ImVec2 a = ImVec2(center.x-10.0f, center.y-10.0f);
        ImVec2 b = ImVec2(center.x+10.0f, center.y+10.0f);
        user_draw_list->AddRectFilled(a, b, color);
    }

    {
        int cr = vdb_sketch_palette[3*vdb_sketch.current_color + 0];
        int cg = vdb_sketch_palette[3*vdb_sketch.current_color + 1];
        int cb = vdb_sketch_palette[3*vdb_sketch.current_color + 2];
        ImU32 color = IM_COL32(cr,cg,cb,255);
        ImVec2 cursor = ImVec2(vdb_sketch.cursor_x, vdb_sketch.cursor_y);
        user_draw_list->AddCircleFilled(cursor, thickness, color);
    }
}
