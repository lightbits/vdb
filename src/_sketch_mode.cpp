enum { SKETCH_MODE_MAX_LINES = 1024*256 };
struct sketch_mode_line_t
{
    ImU32 color;
    bool connected;
    float x1,y1,x2,y2;
};
struct sketch_mode_t
{
    sketch_mode_line_t lines[SKETCH_MODE_MAX_LINES];
    int num_lines;
    bool is_drawing;
    int color_index;
    float cursor_x,cursor_y;
};

namespace sketch_mode
{
    static sketch_mode_t s;

    static int palette[] = {
        IM_COL32((int)(255*1.00f), (int)(255*1.00f), (int)(255*1.00f), 255),
        IM_COL32((int)(255*0.29f), (int)(255*0.50f), (int)(255*0.67f), 255),
        IM_COL32((int)(255*0.71f), (int)(255*0.05f), (int)(255*0.10f), 255),
        IM_COL32((int)(255*0.91f), (int)(255*0.76f), (int)(255*0.41f), 255),
        IM_COL32((int)(255*0.44f), (int)(255*0.64f), (int)(255*0.40f), 255),
        IM_COL32((int)(255*0.10f), (int)(255*0.10f), (int)(255*0.10f), 255),
        IM_COL32((int)(255*0.62f), (int)(255*0.78f), (int)(255*0.89f), 255),
        IM_COL32((int)(255*0.92f), (int)(255*0.60f), (int)(255*0.61f), 255),
        IM_COL32((int)(255*1.00f), (int)(255*0.90f), (int)(255*0.64f), 255),
        IM_COL32((int)(255*0.71f), (int)(255*0.84f), (int)(255*0.67f), 255),
    };

    enum { num_colors = sizeof(palette)/sizeof(palette[0]) };

    static void Update(bool undo_button,
                       bool redo_button,
                       bool clear_button,
                       bool mouse_left_down,
                       float mouse_x,
                       float mouse_y)
    {
        static int redo_num_lines = 0;

        bool &is_drawing = s.is_drawing;
        int &num_lines = s.num_lines;
        ImU32 current_color = palette[s.color_index];
        sketch_mode_line_t *lines = s.lines;

        s.cursor_x = mouse_x;
        s.cursor_y = mouse_y;

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
            if (!is_drawing && num_lines < SKETCH_MODE_MAX_LINES)
            {
                lines[num_lines].x1 = mouse_x;
                lines[num_lines].y1 = mouse_y;
                lines[num_lines].x2 = mouse_x;
                lines[num_lines].y2 = mouse_y;
                lines[num_lines].connected = false;
                lines[num_lines].color = current_color;
                is_drawing = true;
            }
            else if (num_lines < SKETCH_MODE_MAX_LINES)
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
}

