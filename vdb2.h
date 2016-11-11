#ifndef VDB_GL_MAJOR
#define VDB_GL_MAJOR 1
#endif
#ifndef VDB_GL_MINOR
#define VDB_GL_MINOR 5
#endif
#ifndef VDB_MULTISAMPLES
#define VDB_MULTISAMPLES 4
#endif
#ifndef VDB_COLOR_BITS
#define VDB_COLOR_BITS 32
#endif
#ifndef VDB_ALPHA_BITS
#define VDB_ALPHA_BITS 8
#endif
#ifndef VDB_DEPTH_BITS
#define VDB_DEPTH_BITS 24
#endif
#ifndef VDB_STENCIL_BITS
#define VDB_STENCIL_BITS 0
#endif
#ifndef VDB_SETTINGS_FILENAME
#define VDB_SETTINGS_FILENAME "./vdb2.ini"
#endif

#define SO_PLATFORM_IMPLEMENTATION
#define SO_PLATFORM_IMGUI
#define SO_PLATFORM_IMGUI_FONT "C:/Windows/Fonts/Roboto-Bold.ttf", 18.0f
#define SO_NOISE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"
#include "lib/jo_gif.cpp"
#include "lib/imgui/imgui_draw.cpp"
#include "lib/imgui/imgui.cpp"
#include "lib/imgui/imgui_demo.cpp"
#include "lib/so_platform.h"
#include "lib/so_math.h"
#include "lib/so_noise.h"
#include <assert.h>
#define vdb_assert assert
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

    int window_x;
    int window_y;
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
    bool abort;
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

void vdbView3D(mat4 view, mat4 model=m_id4(), float fov=SO_PI/4.0f, float z_near = 0.01f, float z_far = 20.0f)
{
    mat4 projection = mat_perspective(fov, vdb__globals.window_w, vdb__globals.window_h, z_near, z_far);
    vdbView(projection, view, model);
}

mat4 vdbCamera3D(so_input input, vec3 focus = m_vec3(0.0f, 0.0f, 0.0f))
{
    static float radius = 1.0f;
    static float htheta = 0.0f;
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

void vdbGridXY(float x_min, float x_max, float y_min, float y_max, int steps)
{
    for (int i = 0; i <= steps; i++)
    {
        glVertex3f(x_min, y_min + (y_max-y_min)*i/steps, 0.0f);
        glVertex3f(x_max, y_min + (y_max-y_min)*i/steps, 0.0f);

        glVertex3f(x_min + (x_max-x_min)*i/steps, y_min, 0.0f);
        glVertex3f(x_min + (x_max-x_min)*i/steps, y_max, 0.0f);
    }
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

void vdbUnmap(int *i=0, float *x=0, float *y=0, float *z=0)
{
    if (i)
        *i = vdb__globals.map_closest_index;
    if (x)
        *x = vdb__globals.map_closest_x;
    if (y)
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

void vdbSquareViewport()
{
    int w = vdb__globals.window_w;
    int h = vdb__globals.window_h;
    if (w > h)
    {
        vdbViewport((w-h)/2, 0, h, h);
    }
    else
    {
        vdbViewport(0, (h-w)/2, w, w);
    }
}

void vdbClear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void vdbClearViewport(float r, float g, float b, float a)
{
    vdbOrtho(-1,+1,-1,+1);
    glBegin(GL_TRIANGLES);
    glColor4f(r, g, b, a);
    glVertex2f(-1,-1);
    glVertex2f(+1,-1);
    glVertex2f(+1,+1);
    glVertex2f(+1,+1);
    glVertex2f(-1,+1);
    glVertex2f(-1,-1);
    glEnd();
}

#define vdb_Palette_Qualitative_Medium 0
void vdbPalette4i(int i, float a = 1.0f, int palette=vdb_Palette_Qualitative_Medium)
{
    if (palette == vdb_Palette_Qualitative_Medium)
    {
        float c[][3] = {
            { 0.40, 0.76, 0.64 },
            { 0.99, 0.55, 0.38 },
            { 0.54, 0.63, 0.82 },
            { 0.91, 0.54, 0.77 },
            { 0.64, 0.86, 0.29 },
            { 1.00, 0.85, 0.19 },
            { 0.89, 0.77, 0.58 },
            { 0.70, 0.70, 0.70 },
        };
        i = i % 8;
        glColor4f(c[i][0], c[i][1], c[i][2], a);
    }
}

void glPoints(float size) { glPointSize(size); glBegin(GL_POINTS); }
void glLines(float width) { glLineWidth(width); glBegin(GL_LINES); }
void glVertex(vec2 p) { glVertex2f(p.x, p.y); }
void glVertex(vec3 p) { glVertex3f(p.x, p.y, p.z); }
void glTexCoord(vec2 p) { glTexCoord2f(p.x, p.y); }

GLuint vdbTexImage2D(
    void *data,
    int width,
    int height,
    GLenum data_format,
    GLenum data_type = GL_UNSIGNED_BYTE,
    GLenum mag_filter = GL_LINEAR,
    GLenum min_filter = GL_LINEAR,
    GLenum wrap_s = GL_CLAMP_TO_EDGE,
    GLenum wrap_t = GL_CLAMP_TO_EDGE,
    GLenum internal_format = GL_RGBA)
{
    GLuint result = 0;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 internal_format,
                 width,
                 height,
                 0,
                 data_format,
                 data_type,
                 data);
    // glGenerateMipmap(GL_TEXTURE_2D); // todo
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glBindTexture(GL_TEXTURE_2D, 0);
    return result;
}

void vdbSetTexture2D(
    int slot,
    void *data,
    int width,
    int height,
    GLenum data_format,
    GLenum data_type = GL_UNSIGNED_BYTE,
    GLenum mag_filter = GL_LINEAR,
    GLenum min_filter = GL_LINEAR,
    GLenum wrap_s = GL_CLAMP_TO_EDGE,
    GLenum wrap_t = GL_CLAMP_TO_EDGE,
    GLenum internal_format = GL_RGBA)
{
    GLuint tex = 4040 + slot; // Hopefully no one else uses this texture range!
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 internal_format,
                 width,
                 height,
                 0,
                 data_format,
                 data_type,
                 data);
    // glGenerateMipmap(GL_TEXTURE_2D); // todo
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void vdbBindTexture2D(int slot)
{
    glBindTexture(GL_TEXTURE_2D, 4040 + slot);
}

void vdbDrawTexture2D(int slot)
{
    glEnable(GL_TEXTURE_2D);
    vdbBindTexture2D(slot);
    glBegin(GL_TRIANGLES);
    glColor4f(1,1,1,1); glTexCoord2f(0,0); glVertex2f(-1,-1);
    glColor4f(1,1,1,1); glTexCoord2f(1,0); glVertex2f(+1,-1);
    glColor4f(1,1,1,1); glTexCoord2f(1,1); glVertex2f(+1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(1,1); glVertex2f(+1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(0,1); glVertex2f(-1,+1);
    glColor4f(1,1,1,1); glTexCoord2f(0,0); glVertex2f(-1,-1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

struct vdb_settings
{
    int w;
    int h;
    int x;
    int y;
};

vdb_settings vdb_loadSettings()
{
    vdb_settings Result = {0};
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
        Result.w = 640;
        Result.h = 480;
        Result.x = -1;
        Result.y = -1;
    }
    return Result;
}

void vdb_saveSettings()
{
    vdb_settings Settings = {0};
    Settings.w = vdb__globals.window_w;
    Settings.h = vdb__globals.window_h;
    Settings.x = vdb__globals.window_x;
    Settings.y = vdb__globals.window_y;
    FILE *File = fopen(VDB_SETTINGS_FILENAME, "wb+");
    if (File)
    {
        fwrite((const void*)&Settings, sizeof(vdb_settings), 1, File);
        fclose(File);
    }
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
        vdb_settings settings = vdb_loadSettings();
        so_openWindow("vdb", settings.w, settings.h, settings.x, settings.y, VDB_GL_MAJOR, VDB_GL_MINOR, VDB_MULTISAMPLES, VDB_COLOR_BITS, VDB_ALPHA_BITS, VDB_DEPTH_BITS, VDB_STENCIL_BITS);
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
    else
    {
        vdb__globals.step_over = false;
    }

    prev_label = label;
}

void vdb_preamble(so_input input)
{
    vdb__globals.window_x = input.win_x;
    vdb__globals.window_y = input.win_y;
    vdb__globals.window_w = input.width;
    vdb__globals.window_h = input.height;
    vdb__globals.mouse = input.mouse;

    so_imgui_processEvents(input);

    vdbViewport(0, 0, input.width, input.height);
    glClearColor(0.16f, 0.16f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vdbOrtho(-1.0f, +1.0f, -1.0f, +1.0f);

    ImGui::NewFrame();
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

    static jo_gif_t record_gif;
    static int gif_frame_delay = 16;

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

    InputText("Filename", format, sizeof(format));
    RadioButton("Animated GIF", &record_mode, record_mode_gif);
    RadioButton("Image sequence", &record_mode, record_mode_img);
    if (record_mode == record_mode_gif)
        InputInt("Frame delay (ms)", &gif_frame_delay);
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
        if (record_mode == record_mode_gif)
            jo_gif_end(&record_gif);
        recording = false;
    }
    else if (!recording && Button("Start##recording"))
    {
        if (record_mode == record_mode_gif)
        {
            if (record_region)
                record_gif = jo_gif_start(format, (short)(region_right-region_left), (short)(region_top-region_bottom), 0, 32);
            else
                record_gif = jo_gif_start(format, (short)(input.width), (short)(input.height), 0, 32);
        }
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

        unsigned char *data = (unsigned char*)malloc(w*h*4);
        glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

        if (record_mode == record_mode_gif)
        {
            // flip y
            for (int y = 0; y < h/2; y++)
            {
                for (int x = 0; x < w*4; x++)
                {
                    unsigned char temp = data[y*w*4+x];
                    data[y*w*4+x] = data[(h-1-y)*w*4+x];
                    data[(h-1-y)*w*4+x] = temp;
                }
            }
            jo_gif_frame(&record_gif, data, gif_frame_delay/10, false);
        }
        else
        {
            char filename[1024];
            sprintf(filename, format, frame_index);
            int bytes = stbi_write_png(filename, w, h, 4, data+w*(h-1)*4, -w*4);
            if (bytes == 0)
            {
                TextColored(ImVec4(1.0f, 0.3f, 0.1f, 1.0f), "Failed to write file %s\n", filename);
            }
            current_bytes += bytes;
        }
        frame_index++;
        free(data);

        if (frame_limit > 0 && frame_index >= frame_limit)
        {
            if (record_mode == record_mode_gif)
                jo_gif_end(&record_gif);
            recording = false;
        }

    }

    End();

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

    static float osd_timer_interval = 0.18f;
    static float osd_timer = osd_timer_interval;
    const int osd_mode_closed = 0;
    const int osd_mode_opening = 1;
    const int osd_mode_opened = 2;
    const int osd_mode_closing = 3;
    static int osd_mode = 0;
    static bool osd_show_ruler_tool = false;
    static bool open_video_tool = false;
    const float escape_timer_duration = 0.5f;
    static float escape_timer = escape_timer_duration;

    if (input.keys[SO_KEY_F4].pressed && input.keys[SO_KEY_ALT].down)
    {
        vdb__globals.abort = true;
    }

    if (input.keys[SO_KEY_ESCAPE].down)
    {
        escape_timer -= input.dt;
        float t = escape_timer/escape_timer_duration;
        float x0 = 0.0f;
        float x1 = t*input.width;
        float y0 = 0.0f;
        float y1 = 5.0f;
        float a = m_map(0.2f, 0.5f, 1.0f-t, 0.0f, 0.75f);
        a *= a;
        vdbOrtho(0.0f, input.width, input.height, 0.0f);
        glBegin(GL_TRIANGLES);
        glColor4f(0.3f, 0.3f, 0.3f, a);
        glVertex2f(x0, y0);
        glVertex2f(x1, y0);
        glVertex2f(x1, y1);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glVertex2f(x0, y0);
        glEnd();
        if (escape_timer < 0.0f)
            vdb__globals.abort = true;
    }
    else
    {
        escape_timer = escape_timer_duration;
    }

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

        float a = powf(t, 0.2f);
        float w = 0.35f;
        float p = 0.0f;

        SetNextWindowPos(ImVec2(-w*input.width + (w*input.width+p)*a, p));
        SetNextWindowSize(ImVec2(w*input.width, input.height-2.0f*p));
        PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
        PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
        PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
        PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
        PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
        Begin("##vdb_osd", 0,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse);

        // flow control
        bool hotkey_screenshot = input.keys[SO_KEY_PRINTSCREEN].pressed;
        bool hotkey_ruler = input.keys[SO_KEY_R].pressed && input.keys[SO_KEY_CTRL].down;
        bool hotkey_video = input.keys[SO_KEY_V].pressed && input.keys[SO_KEY_CTRL].down;
        bool hotkey_size = input.keys[SO_KEY_W].pressed && input.keys[SO_KEY_CTRL].down;

        {
            if (Button("Step once")) vdb__globals.step_once = true;
            if (Button("Step over")) vdb__globals.step_over = true;
            if (Button("Step and skip")) vdb__globals.step_skip = true;
            if (Button("Take screenshot") || input.keys[SO_KEY_PRINTSCREEN].pressed)
            {
                ImGui::OpenPopup("Take screenshot##popup");
            }
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
            if (osd_show_ruler_tool && (Button("Hide ruler") || hotkey_ruler))
            {
                osd_show_ruler_tool = false;
            }
            else if (!osd_show_ruler_tool && (Button("Show ruler") || hotkey_ruler))
            {
                osd_show_ruler_tool = true;
            }
            if (Button("Set window size") || hotkey_size)
            {
                ImGui::OpenPopup("Set window size##popup");
            }
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
            if (Button("Record video") || hotkey_video)
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
                    while (true) {                      \
                    if (!so_loopWindow(&vdb_input) ||   \
                        vdb__globals.abort) {           \
                        vdb_saveSettings();             \
                        exit(1);                        \
                    }                                   \
                    if (vdb__globals.break_loop) break; \
                    using namespace ImGui;              \
                    vdb_preamble(vdb_input);
                    // so_input input = vdb_input;

#define VDBE()      vdb_postamble(vdb_input); } }

