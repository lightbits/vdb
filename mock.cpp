#define SO_PLATFORM_IMPLEMENTATION
#define SO_PLATFORM_IMGUI
#define SO_PLATFORM_IMGUI_FONT "C:/Windows/Fonts/Roboto-Bold.ttf", 18.0f
#define SO_NOISE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"
#include "lib/imgui/imgui_draw.cpp"
#include "lib/imgui/imgui.cpp"
#include "lib/imgui/imgui_demo.cpp"
#include "lib/so_platform.h"
#include "lib/so_math.h"
#include "lib/so_noise.h"
#define vdb_countof(X) (sizeof(X) / sizeof((X)[0]))
#define vdb_for(VAR, FIRST, LAST_PLUS_ONE) for (int VAR = FIRST; VAR < LAST_PLUS_ONE; VAR++)
#define vdbKeyDown(KEY) input.keys[SO_KEY_##KEY].down
#define vdbKeyPressed(KEY) input.keys[SO_KEY_##KEY].pressed
#define vdbKeyReleased(KEY) input.keys[SO_KEY_##KEY].released

/*
Press esc (or tab): Open VDB overlay.
Recording options, etc. Smooth animation?

vdb_mapPoint2(i, x, y):
vdb_mapPoint3(i, x, y, z):

vdb_getHoveredElement(): index
Works with 3D and 2D projections

vdbView2D
vdbView3D

vdb_forEach2f()
{
    x, y, i is accessible in here.
    Default behaviour is that this loop only iterates over one element:
    The one hovered over by the mouse.

    If the tools menu is uncovered, and the user presses the "Select multi"
    button, this for loop may iterate over several elements.
}

vdb_ndc_to_view
vdb_view_to_ndc
vdb_ndc_to_world
vdb_world_to_ndc
*/

static struct vdb_globals
{
    int viewport_x;
    int viewport_y;
    int viewport_w;
    int viewport_h;

    int window_w;
    int window_h;

    mat4 pvm;

    int map_index;
    int map_closest_index;
    float map_closest_distance;
    float map_closest_x;
    float map_closest_y;
    float map_closest_z;

    so_input_mouse mouse;

    bool step_once;
    bool step_over;
    bool step_skip;
    bool break_loop;
} vdb__globals;

void vdbNDCToWindow(float x, float y, float *wx, float *wy)
{
    *wx = (0.5f + 0.5f*x)*vdb__globals.window_w;
    *wy = (0.5f - 0.5f*y)*vdb__globals.window_h;
}

void vdbWindowToNDC(float x, float y, float *nx, float *ny)
{
    *nx = -1.0f + 2.0f * x / vdb__globals.window_w;
    *ny = +1.0f - 2.0f * y / vdb__globals.window_h;
}

void vdbModelToNDC(float x, float y, float z, float w, float *x_ndc, float *y_ndc)
{
    vec4 model = { x, y, z, w };
    vec4 clip = vdb__globals.pvm * model;
    *x_ndc = clip.x / clip.w;
    *y_ndc = clip.y / clip.w;
}

void vdbModelToWindow(float x, float y, float z, float w, float *x_win, float *y_win)
{
    float x_ndc, y_ndc;
    vdbModelToNDC(x, y, z, w, &x_ndc, &y_ndc);
    vdbNDCToWindow(x_ndc, y_ndc, x_win, y_win);
}

void vdbView(mat4 projection, mat4 view, mat4 model)
{
    mat4 pvm = projection * view * model;
    vdb__globals.pvm = pvm;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(pvm.data);
}

mat4 vdbCamera3D(so_input input, vec3 focus = m_vec3(0.0f, 0.0f, 0.0f))
{
    static float radius = 1.0f;
    static float htheta = SO_PI/2.0f-0.3f;
    static float vtheta = 0.3f;
    static float Rradius = radius;
    static float Rhtheta = htheta;
    static float Rvtheta = vtheta;

    float dt = input.dt;
    if (vdbKeyDown(SHIFT))
    {
        if (vdbKeyPressed(Z))
            Rradius /= 2.0f;
        if (vdbKeyPressed(X))
            Rradius *= 2.0f;
        if (vdbKeyPressed(LEFT))
            Rhtheta -= SO_PI / 4.0f;
        if (vdbKeyPressed(RIGHT))
            Rhtheta += SO_PI / 4.0f;
        if (vdbKeyPressed(UP))
            Rvtheta -= SO_PI / 4.0f;
        if (vdbKeyPressed(DOWN))
            Rvtheta += SO_PI / 4.0f;
    }
    else
    {
        if (vdbKeyDown(Z))
            Rradius -= dt;
        if (vdbKeyDown(X))
            Rradius += dt;
        if (vdbKeyDown(LEFT))
            Rhtheta -= dt;
        if (vdbKeyDown(RIGHT))
            Rhtheta += dt;
        if (vdbKeyDown(UP))
            Rvtheta -= dt;
        if (vdbKeyDown(DOWN))
            Rvtheta += dt;
    }

    radius += 10.0f * (Rradius - radius) * dt;
    htheta += 10.0f * (Rhtheta - htheta) * dt;
    vtheta += 10.0f * (Rvtheta - vtheta) * dt;

    mat3 R = m_mat3(mat_rotate_z(htheta)*mat_rotate_x(vtheta));
    vec3 p = focus + R.a3 * radius;
    mat4 c_to_w = m_se3(R, p);
    return m_se3_inverse(c_to_w);
}

void vdbOrtho(float left, float right, float bottom, float top)
{
    float ax = 2.0f/(right-left);
    float ay = 2.0f/(top-bottom);
    float bx = (left+right)/(left-right);
    float by = (bottom+top)/(bottom-top);
    mat4 projection = {
        ax, 0.0f, 0.0f, 0.0f,
        0.0f, ay, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        bx, by, 0.0f, 1.0f
    };
    mat4 view = m_id4();
    mat4 model = m_id4();
    vdbView(projection, view, model);
}

void vdbBeginMap()
{
    float w = vdb__globals.window_w;
    float h = vdb__globals.window_h;
    vdb__globals.map_closest_distance = w*w + h*h;
    vdb__globals.map_index = 0;
}

void vdbMap(float x, float y, float z = 0.0f, float w = 1.0f)
{
    // todo: find closest in z

    vec4 model = { x, y, z, w };
    vec4 clip = vdb__globals.pvm * model;
    float x_ndc = clip.x / clip.w; // todo: verify?
    float y_ndc = clip.y / clip.w; // todo: verify?
    // float z_ndc = clip.z / clip.w; // todo: verify?
    float x_win = (0.5f+0.5f*x_ndc)*vdb__globals.window_w;
    float y_win = (0.5f-0.5f*y_ndc)*vdb__globals.window_h;

    float dx = x_win - vdb__globals.mouse.x;
    float dy = y_win - vdb__globals.mouse.y;

    float distance = dx*dx + dy*dy;

    if (distance < vdb__globals.map_closest_distance)
    {
        vdb__globals.map_closest_index = vdb__globals.map_index;
        vdb__globals.map_closest_x = x;
        vdb__globals.map_closest_y = y;
        vdb__globals.map_closest_distance = distance;
    }

    vdb__globals.map_index++;
}

void vdbUnmap(int *i, float *x, float *y, float *z=0)
{
    *i = vdb__globals.map_closest_index;
    *x = vdb__globals.map_closest_x;
    *y = vdb__globals.map_closest_y;
    // if (z)
        // vdb__globals.map_closest_z;
}

void vdbFillCircle(float x, float y, float r, int n = 16)
{
    for (int ti = 0; ti < n; ti++)
    {
        float t1 = 2.0f*3.1415926f*(ti+0)/n;
        float t2 = 2.0f*3.1415926f*(ti+1)/n;
        glVertex2f(x, y);
        glVertex2f(x+r*cosf(t1), y+r*sinf(t1));
        glVertex2f(x+r*cosf(t2), y+r*sinf(t2));
    }
}

void vdbDrawCircle(float x, float y, float r, int n = 16)
{
    for (int ti = 0; ti < n; ti++)
    {
        float t1 = 2.0f*3.1415926f*(ti+0)/n;
        float t2 = 2.0f*3.1415926f*(ti+1)/n;
        glVertex2f(x+r*cosf(t1), y+r*sinf(t1));
        glVertex2f(x+r*cosf(t2), y+r*sinf(t2));
    }
}

void vdbViewport(int x, int y, int w, int h)
{
    vdb__globals.viewport_x = x;
    vdb__globals.viewport_y = y;
    vdb__globals.viewport_w = w;
    vdb__globals.viewport_h = h;
    glViewport(x, y, w, h);
}

void vdb_init(const char *label)
{
    struct window
    {
        const char *label;
        bool hidden;
    };
    static window windows[1024];
    static int window_count = 0;

    static const char *prev_label = 0;
    static bool have_window = false;
    if (!have_window)
    {
        so_openWindow("vdb", 640, 480, -1, -1, 1, 5, 4, 32, 8, 24, 0);
        so_imgui_init();
        have_window = true;
    }

    vdb__globals.break_loop = false;
    vdb__globals.step_once = false;

    if (prev_label == label) // is same visualization
    {
        if (vdb__globals.step_over)
            vdb__globals.break_loop = true;
    }

    prev_label = label;
}

void vdb_preamble(so_input input)
{
    vdb__globals.window_w = input.width;
    vdb__globals.window_h = input.height;
    vdb__globals.mouse = input.mouse;

    so_imgui_processEvents(input);

    vdbViewport(0, 0, input.width, input.height);
    glClearColor(0.16f, 0.16f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ImGui::NewFrame();
}

void vdb_osd_push_tool_style()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.96f, 0.96f, 0.96f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.9f, 0.9f, 0.9f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.9f, 0.9f, 0.9f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.9f, 0.9f, 0.9f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_CloseButton, ImVec4(0.9f, 0.9f, 0.9f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_CloseButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_CloseButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8f, 0.8f, 0.8f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.8f, 0.8f, 0.8f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.8f, 0.8f, 0.8f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.96f, 0.96f, 0.96f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.8f, 0.8f, 0.8f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.8f, 0.8f, 0.8f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.8f, 0.8f, 0.8f, 2.0f));
}

void vdb_osd_pop_tool_style()
{
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(20);
}

void vdb_osd_ruler_tool(so_input input)
{
    using namespace ImGui;
    static float x1_ndc = -0.2f;
    static float y1_ndc = -0.2f;
    static float x2_ndc = +0.2f;
    static float y2_ndc = +0.2f;
    const float grab_radius = 16.0f;
    static float *grabbed_x = 0;
    static float *grabbed_y = 0;

    float x1, y1, x2, y2;
    vdbNDCToWindow(x1_ndc, y1_ndc, &x1, &y1);
    vdbNDCToWindow(x2_ndc, y2_ndc, &x2, &y2);

    // mouse grabbing
    if (input.left.pressed)
    {
        float dx1 = x1 - input.mouse.x;
        float dy1 = y1 - input.mouse.y;
        float d1 = sqrtf(dx1*dx1 + dy1*dy1);
        if (d1 < grab_radius)
        {
            grabbed_x = &x1_ndc;
            grabbed_y = &y1_ndc;
        }

        float dx2 = x2 - input.mouse.x;
        float dy2 = y2 - input.mouse.y;
        float d2 = sqrtf(dx2*dx2 + dy2*dy2);
        if (d2 < grab_radius)
        {
            grabbed_x = &x2_ndc;
            grabbed_y = &y2_ndc;
        }
    }
    if (input.left.released)
    {
        grabbed_x = 0;
        grabbed_y = 0;
    }

    if (grabbed_x && grabbed_y)
    {
        *grabbed_x = input.mouse.u;
        *grabbed_y = input.mouse.v;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoInputs;

    float x_left = (x1 < x2) ? x1 : x2;
    float x_right = (x1 > x2) ? x1 : x2;
    float y_bottom = (y1 > y2) ? y1 : y2;
    float y_top = (y1 < y2) ? y1 : y2;
    float padding = 128.0f;

    char text[256];
    float distance_px = sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    sprintf(text, "%.2f px", distance_px);

    SetNextWindowPos(ImVec2(x_left-padding, y_top-padding));
    SetNextWindowSize(ImVec2(x_right-x_left+2.0f*padding, y_bottom-y_top+2.0f*padding));
    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    Begin("##vdb_selection_tool_0",0, flags);
    ImDrawList *dl = GetWindowDrawList();

    // draw target circles
    const float handle_radius = 5.0f;
    dl->AddCircleFilled(ImVec2(x1, y1+1), handle_radius, 0x99000000, 32);
    dl->AddCircleFilled(ImVec2(x1, y1), handle_radius, 0xffffffff, 32);
    dl->AddCircleFilled(ImVec2(x2, y2+1), handle_radius, 0x99000000, 32);
    dl->AddCircleFilled(ImVec2(x2, y2), handle_radius, 0xffffffff, 32);

    // draw connecting line
    dl->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), 0xffffffff, 1.0f);

    // draw distance text
    dl->AddText(ImVec2((x1+x2)*0.5f+1.0f, (y1+y2)*0.5f+1.0f), 0xbb000000, text);
    dl->AddText(ImVec2((x1+x2)*0.5f, (y1+y2)*0.5f), 0xffffffff, text);
    End();
    PopStyleColor();

    // SetNextWindowPos(ImVec2(x1, y1));
    // PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
    // Begin("##vdb_selection_tool_1", 0, flags);
    // Text("%.2f %.2f", x1, y1);
    // End();

    // SetNextWindowPos(ImVec2(x2, y2));
    // Begin("##vdb_selection_tool_2", 0, flags);
    // Text("%.2f %.2f", x2, y2);
    // End();
    // PopStyleColor();
}

void vdb_osd_video_tool(bool *open_video_tool, so_input input)
{
    using namespace ImGui;
    static char format[1024];

    const int record_mode_gif = 0;
    const int record_mode_img = 1;
    static int record_mode = record_mode_img;

    static bool record_region = false;
    static int region_left = 0;
    static int region_right = input.width;
    static int region_bottom = 0;
    static int region_top = input.height;

    static int frame_limit = 0;
    static bool recording = false;
    static int frame_index = 0;
    static unsigned long long current_bytes = 0;

    vdb_osd_push_tool_style();
    if (current_bytes > 0)
    {
        float megabytes = current_bytes / (1024.0f*1024.0f);
        char title[256];
        sprintf(title, "Record video (%d frames, %.2f mb)###Record video", frame_index, megabytes);
        Begin(title, open_video_tool);
    }
    else
    {
        Begin("Record video###Record video", open_video_tool);
    }

    InputText("Format", format, sizeof(format));
    RadioButton("Animated GIF", &record_mode, record_mode_gif);
    RadioButton("Image sequence", &record_mode, record_mode_img);
    // if (record_mode == record_mode_gif)
        // InputInt("Frame delay", )
    Separator();
    InputInt("Frames", &frame_limit);
    Separator();
    Checkbox("Record region", &record_region);
    if (record_region)
    {
        SliderInt("left##record_region", &region_left, 0, input.width);
        SliderInt("right##record_region", &region_right, 0, input.width);
        SliderInt("bottom##record_region", &region_bottom, 0, input.height);
        SliderInt("top##record_region", &region_top, 0, input.height);
        Text("%d x %d", region_right-region_left, region_top-region_bottom);
    }
    Separator();
    if (recording && Button("Stop##recording"))
    {
        recording = false;
    }
    else if (!recording && Button("Start##recording"))
    {
        frame_index = 0;
        current_bytes = 0;
        recording = true;
    }
    SameLine();

    if (recording)
    {
        int x, y, w, h;
        if (record_region)
        {
            x = region_left;
            y = region_bottom;
            w = region_right-region_left;
            h = region_top-region_bottom;
        }
        else
        {
            x = 0;
            y = 0;
            w = input.width;
            h = input.height;
        }

        // Specify the start of each pixel row in memory to be 1-byte aligned
        // as opposed to 4-byte aligned or something.
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_BACK);

        unsigned char *data = (unsigned char*)malloc(w*h*3);
        glReadPixels(x, y, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);

        char filename[1024];
        // sprintf(filename, format, frame_index);
        sprintf(filename, "C:/Temp/out_data_3aug_2/video%04d.png", frame_index);
        int bytes = stbi_write_png(filename, w, h, 3, data+w*(h-1)*3, -w*3);
        if (bytes == 0)
        {
            TextColored(ImVec4(1.0f, 0.3f, 0.1f, 1.0f), "Failed to write file %s\n", filename);
        }
        current_bytes += bytes;
        frame_index++;
        free(data);

        if (frame_limit > 0 && frame_index >= frame_limit)
        {
            recording = false;
        }

    }

    End();
    vdb_osd_pop_tool_style();

    if (record_region)
    {
        vdbOrtho(0.0f, input.width, 0.0f, input.height);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
        glVertex2f(region_left+1.0f, region_bottom-1.0f);
        glVertex2f(region_right+1.0f, region_bottom-1.0f);
        glVertex2f(region_right+1.0f, region_top-1.0f);
        glVertex2f(region_left+1.0f, region_top-1.0f);
        glVertex2f(region_left+1.0f, region_bottom-1.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glVertex2f(region_left, region_bottom);
        glVertex2f(region_right, region_bottom);
        glVertex2f(region_right, region_top);
        glVertex2f(region_left, region_top);
        glVertex2f(region_left, region_bottom);
        glEnd();
    }
}

void vdb_postamble(so_input input)
{
    vdbOrtho(-1.0f, +1.0f, -1.0f, +1.0f);

    using namespace ImGui;

    static float osd_timer_interval = 0.15f;
    static float osd_timer = osd_timer_interval;
    const int osd_mode_closed = 0;
    const int osd_mode_opening = 1;
    const int osd_mode_opened = 2;
    const int osd_mode_closing = 3;
    static int osd_mode = 0;
    static bool osd_show_ruler_tool = false;
    static bool open_video_tool = false;

    if (input.keys[SO_KEY_ESCAPE].pressed)
    {
        if (osd_mode == osd_mode_closed)
        {
            osd_timer = osd_timer_interval;
            osd_mode = osd_mode_opening;
        }
        else
        if (osd_mode == osd_mode_opened)
        {
            osd_timer = osd_timer_interval;
            osd_mode = osd_mode_closing;
        }
        else
        if (osd_mode == osd_mode_opening)
        {
            osd_timer = osd_timer_interval - osd_timer;
            osd_mode = osd_mode_closing;
        }
        else
        if (osd_mode == osd_mode_closing)
        {
            osd_timer = osd_timer_interval - osd_timer;
            osd_mode = osd_mode_opening;
        }
    }

    if (osd_mode == osd_mode_opening ||
        osd_mode == osd_mode_opened ||
        osd_mode == osd_mode_closing)
    {
        if (osd_timer > 0.0f)
            osd_timer -= input.dt;

        if (osd_timer <= 0.0f)
        {
            if (osd_mode == osd_mode_opening)
                osd_mode = osd_mode_opened;
            else if (osd_mode == osd_mode_closing)
                osd_mode = osd_mode_closed;
        }

        float t = 0.0f;
        if (osd_mode == osd_mode_closed)
            t = 0.0f;
        if (osd_mode == osd_mode_opening)
            t = 1.0f - osd_timer / osd_timer_interval;
        if (osd_mode == osd_mode_closing)
            t = osd_timer / osd_timer_interval;
        if (osd_mode == osd_mode_opened)
            t = 1.0f;

        float a = powf(t, 0.13f);
        float w = 0.35f;

        SetNextWindowPos(ImVec2(-w*input.width + (w*input.width+10.0f)*a, 10.0f));
        SetNextWindowSize(ImVec2(w*input.width, input.height-20.0f));
        PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
        PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
        PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
        PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
        PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.96f, 0.96f, 0.96f, 0.0f));
        Begin("##vdb_osd", 0,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse);

        // flow control
        {
            if (Button("Step once")) vdb__globals.step_once = true;
            if (Button("Step over")) vdb__globals.step_over = true;
            if (Button("Step and skip")) vdb__globals.step_skip = true;
            if (Button("Take screenshot"))
            {
                ImGui::OpenPopup("Take screenshot##popup");
            }
            vdb_osd_push_tool_style();
            if (BeginPopupModal("Take screenshot##popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                static char filename[1024];
                InputText("Filename", filename, sizeof(filename));

                if (Button("OK", ImVec2(120,0)) || input.keys[SO_KEY_ENTER].pressed)
                {
                    unsigned char *data = (unsigned char*)malloc(input.width*input.height*3);
                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                    glReadBuffer(GL_BACK);
                    glReadPixels(0, 0, input.width, input.height, GL_RGB, GL_UNSIGNED_BYTE, data);
                    stbi_write_png(filename, input.width, input.height, 3, data+input.width*(input.height-1)*3, -input.width*3);
                    free(data);
                    CloseCurrentPopup();
                }
                SameLine();
                if (Button("Cancel", ImVec2(120,0))) { CloseCurrentPopup(); }
                EndPopup();
            }
            vdb_osd_pop_tool_style();
            if (osd_show_ruler_tool && Button("Hide ruler"))
            {
                osd_show_ruler_tool = false;
            }
            else if (!osd_show_ruler_tool && Button("Show ruler"))
            {
                osd_show_ruler_tool = true;
            }
            if (Button("Set window size"))
            {
                ImGui::OpenPopup("Set window size##popup");
            }
            vdb_osd_push_tool_style();
            if (BeginPopupModal("Set window size##popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                static int width = input.width;
                static int height = input.height;
                static bool topmost = false;
                InputInt("Width", &width);
                InputInt("Height", &height);
                Separator();
                Checkbox("Topmost", &topmost);

                if (Button("OK", ImVec2(120,0)) || input.keys[SO_KEY_ENTER].pressed)
                {
                    so_setWindowPos(0, 0, width, height, topmost);
                    CloseCurrentPopup();
                }
                SameLine();
                if (Button("Cancel", ImVec2(120,0))) { CloseCurrentPopup(); }
                EndPopup();
            }
            vdb_osd_pop_tool_style();
            if (Button("Record video"))
            {
                open_video_tool = true;
            }
        }

        End();
        PopStyleColor();
        PopStyleColor();
        PopStyleColor();
        PopStyleColor();
        PopStyleColor();
        PopStyleVar();
        PopStyleVar();
        PopStyleVar();
    }

    if (osd_show_ruler_tool)
        vdb_osd_ruler_tool(input);

    if (open_video_tool)
        vdb_osd_video_tool(&open_video_tool, input);

    Render();
    so_swapBuffers();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        assert(false);

    if (input.keys[SO_KEY_F10].pressed)
    {
        if (input.keys[SO_KEY_SHIFT].down)
            vdb__globals.step_skip = true;
        else
            vdb__globals.step_once = true;
    }
    if (input.keys[SO_KEY_F5].pressed) vdb__globals.step_over = true;

    if (vdb__globals.step_once)
    {
        vdb__globals.break_loop = true;
    }
    if (vdb__globals.step_skip)
    {
        vdb__globals.break_loop = true;
    }
    if (vdb__globals.step_over)
    {
        vdb__globals.break_loop = true;
    }
}

#define VDBB(LABEL) { vdb_init(LABEL);                  \
                    so_input vdb_input = {0};           \
                    while (so_loopWindow(&vdb_input)) { \
                    if (vdb__globals.break_loop) break; \
                    using namespace ImGui;              \
                    vdb_preamble(vdb_input);            \
                    so_input input = vdb_input;

#define VDBE()      vdb_postamble(vdb_input); } }

int main()
{
    struct Thing
    {
        float error;
        int count;
    };

    #define N 32
    Thing things[N*N];
    vdb_for(x, 0, N)
    vdb_for(y, 0, N)
    {
        things[y*N+x].count = y*N+x;
        things[y*N+x].error = x*y;
    }

    VDBB("my label");
    {
        vdbView(mat_perspective(SO_PI/4.0f, input.width, input.height, 0.01f, 10.0f),
                vdbCamera3D(input), m_id4());
        vdbBeginMap();
        glBegin(GL_TRIANGLES);
        {
            int nx = (int)(input.width/64.0f);
            int ny = (int)(input.height/64.0f);
            vdb_for(yi, 0, ny+1)
            vdb_for(xi, 0, nx+1)
            {
                float xt = (float)xi/nx;
                float yt = (float)yi/ny;
                float xn = -1.0f + 2.0f*xt;
                float yn = -1.0f + 2.0f*yt;
                float dx = 2.0f/nx;
                float dy = 2.0f/ny;
                glColor4f(0.2f+0.8f*xt, 0.3f+0.7f*yt, 0.5f+0.5f*sinf(0.3f*input.t), 1.0f);
                glVertex2f(xn, yn);
                glVertex2f(xn+dx, yn);
                glVertex2f(xn+dx, yn+dy);
                glVertex2f(xn+dx, yn+dy);
                glVertex2f(xn, yn+dy);
                glVertex2f(xn, yn);

                vdbMap(xn+1.0f/nx, yn+1.0f/ny);
            }
        }
        glEnd();

        {
            int i;
            float x_src, y_src;
            vdbUnmap(&i, &x_src, &y_src);
            SetTooltip("%d: %.2f %.2f", i, x_src, y_src);

            float x_win, y_win;
            vdbModelToWindow(x_src, y_src, 0.0f, 1.0f, &x_win, &y_win);
            vdbOrtho(0.0f, input.width, input.height, 0.0f);
            glBegin(GL_TRIANGLES);
            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            vdbFillCircle(x_win+1.0f, y_win+1.0f, 5.0f);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            vdbFillCircle(x_win, y_win, 5.0f);
            glEnd();
        }

        // vdb_pushView
        // vdb_steerableView2D()
        // vdb_view2D()
        // vdb_for(x, 0, N)
        // vdb_for(y, 0, N)
        // {
        //     int i = y*N+x;
        //     vdb_map2f(i, x, y);
        // }

        // {
        //     float x, y;
        //     int i = vdb_unmap2f(&x, &y);
        //     SetTooltip("count: %d\nerror: %.2f", things[i].count, things[i].error);
        // }

        // Begin("Test");
        // ShowTestWindow();
        // End();
    }
    VDBE();

    VDBB("Ho");
    {
        vdbOrtho(-1.0f, +1.0f, -1.0f, +1.0f);
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        {
            mat3 R = m_mat3(mat_rotate_z(0.4f)*mat_rotate_y(0.4f)*mat_rotate_x(0.4f));
            int n = 16;
            vdb_for(zi, 0, n+1)
            vdb_for(yi, 0, n+1)
            vdb_for(xi, 0, n+1)
            {
                float xt = (float)xi/n;
                float yt = (float)yi/n;
                float zt = (float)zi/n;
                int it = xi*n*n+yi*n+zi;
                float offx = sinf(3.0f*input.t+0.0f+it)*(-1.0f+2.0f*noise_hash1f(it))/(4.0f*n);
                float offy = sinf(3.0f*input.t+1.5f+it)*(-1.0f+2.0f*noise_hash1f(it))/(4.0f*n);
                float offz = sinf(3.0f*input.t+3.1f+it)*(-1.0f+2.0f*noise_hash1f(it))/(4.0f*n);
                vec3 pm = m_normalize(m_vec3(-1.0f+2.0f*xt, -1.0f+2.0f*yt, -1.0f+2.0f*zt));
                pm += m_vec3(offx, offy, offz);
                vec3 pc = R*pm + m_vec3(0.0f, 0.0f, -5.0f);
                float f = (input.height/2.0f) / tanf(SO_PI/8.0f);
                vec2 uv = m_project_pinhole(f, f, input.width/2.0f, input.height/2.0f, pc);
                float xn = -1.0f + 2.0f*uv.x/input.width;
                float yn = -1.0f + 2.0f*uv.y/input.height;
                glColor4f(0.9f+0.1f*xt, 0.8f+0.2f*yt, 0.85f+0.15f*zt, 1.0f);
                glVertex2f(xn, yn);
            }
        }
        glEnd();
    }
    VDBE();
}
