#define SO_PLATFORM_IMPLEMENTATION
#define SO_PLATFORM_IMGUI
#define SO_PLATFORM_IMGUI_FONT "C:/Windows/Fonts/Roboto-Bold.ttf", 18.0f
#define SO_NOISE_IMPLEMENTATION
#include "lib/imgui/imgui_draw.cpp"
#include "lib/imgui/imgui.cpp"
#include "lib/imgui/imgui_demo.cpp"
#include "lib/so_platform.h"
#include "lib/so_math.h"
#include "lib/so_noise.h"
#define vdb_countof(X) (sizeof(X) / sizeof((X)[0]))
#define vdb_for(VAR, FIRST, LAST_PLUS_ONE) for (int VAR = FIRST; VAR < LAST_PLUS_ONE; VAR++)

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

void vdbViewport(int x, int y, int w, int h)
{
    vdb__globals.viewport_x = x;
    vdb__globals.viewport_y = y;
    vdb__globals.viewport_w = w;
    vdb__globals.viewport_h = h;
    glViewport(x, y, w, h);
}

struct vdb_state
{
    const char *label;
};

vdb_state vdb_init(const char *label)
{
    static bool have_window = false;
    if (!have_window)
    {
        so_openWindow("vdb", 640, 480, -1, -1, 1, 5, 0, 32, 8, 24, 0);
        so_imgui_init();
    }

    vdb_state result = {0};
    result.label = label;
    return result;
}

void vdb_preamble(so_input input, vdb_state *state)
{
    vdb__globals.window_w = input.width;
    vdb__globals.window_h = input.height;

    so_imgui_processEvents(input);

    vdbViewport(0, 0, input.width, input.height);
    glClearColor(0.16f, 0.16f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ImGui::NewFrame();
}

void vdb_osd_ruler_tool(so_input input, vdb_state *state)
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
    float padding = 64.0f;

    char text[256];
    float distance_px = sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    float distance_ndc = sqrtf((x2_ndc-x1_ndc)*(x2_ndc-x1_ndc) + (y2_ndc-y1_ndc)*(y2_ndc-y1_ndc));
    sprintf(text, "%.2f px", distance_px);
    // sprintf(text, "%.2f px\n%.2f ndc\n%.2f units", distance_px, distance_ndc, 2.0f);

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

void vdb_postamble(so_input input, vdb_state *state)
{
    // todo: vdbView(-1, +1, -1, +1)

    using namespace ImGui;

    static float osd_timer_interval = 0.15f;
    static float osd_timer = osd_timer_interval;
    const int osd_mode_closed = 0;
    const int osd_mode_opening = 1;
    const int osd_mode_opened = 2;
    const int osd_mode_closing = 3;
    static int osd_mode = 0;
    static bool osd_show_ruler_tool = false;

    if (input.keys[SO_KEY_TAB].pressed)
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
            if (Button("Step once"))
            {

            }
            if (Button("Step over"))
            {

            }
            if (Button("Skip"))
            {

            }
            if (Button("Take screenshot"))
            {

            }
            if (osd_show_ruler_tool && Button("Hide ruler"))
            {
                osd_show_ruler_tool = false;
            }
            else if (!osd_show_ruler_tool && Button("Show ruler"))
            {
                osd_show_ruler_tool = true;
            }
            if (osd_show_ruler_tool)
            {
                SameLine();
                PushItemWidth(-1.0f);
                static int item = 1;
                ImGui::Combo("combo", &item, "Pixels\0NDC\0View\0World\0\0");
            }
            {

            }
            if (Button("Record video"))
            {

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
        vdb_osd_ruler_tool(input, state);

    Render();
    so_swapBuffers();
}

#define VDBB(LABEL) vdb_state vdb_##__LINE__##_state = vdb_init(LABEL);     \
                    so_input vdb_##__LINE__##_input = {0};                  \
                    while (so_loopWindow(&vdb_##__LINE__##_input)) {        \
                    using namespace ImGui;                                  \
                    vdb_preamble(vdb_##__LINE__##_input, &vdb_##__LINE__##_state); \
                    so_input input = vdb_##__LINE__##_input;

#define VDBE()      vdb_postamble(vdb_##__LINE__##_input, &vdb_##__LINE__##_state); }

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
        // glBegin(GL_LINES);
        // for (int i = 0; i < 256; i++)
        // {
        //     float t1 = 2.0f*3.1415f*(i+0)/256.0f;
        //     float t2 = 2.0f*3.1415f*(i+1)/256.0f;
        //     float r1 = cos(8.0f*t1) + 0.1f*sin(input.t);
        //     float r2 = cos(8.0f*t2) + 0.1f*sin(input.t);
        //     float x1 = r1*cos(t1 + 0.1f*input.t);
        //     float y1 = r1*sin(t1 + 0.1f*input.t);
        //     float x2 = r2*cos(t2 + 0.1f*input.t);
        //     float y2 = r2*sin(t2 + 0.1f*input.t);
        //     glVertex2f(x1, y1);
        //     glVertex2f(x2, y2);
        // }
        // glEnd();

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
            }
        }
        glEnd();

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
}
