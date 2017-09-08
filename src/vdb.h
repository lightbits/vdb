#ifndef VBD_HEADER
#define VBD_HEADER

// The following are settings that can be overwritten by #defining
// any of the following variables before including the implementation
// of this file.

// Set to > 0 to get smooth edges on points, lines and triangles.
#ifndef VDB_MULTISAMPLES
#define VDB_MULTISAMPLES 4
#endif

// Set to > 0 to be able to take screenshots with transparent backgrounds.
#ifndef VDB_ALPHA_BITS
#define VDB_ALPHA_BITS 8
#endif

// Set to > 0 if you want to use OpenGL depth testing.
#ifndef VDB_DEPTH_BITS
#define VDB_DEPTH_BITS 24
#endif

// Set to > 0 if you want to use the OpenGL stencil operations.
#ifndef VDB_STENCIL_BITS
#define VDB_STENCIL_BITS 0
#endif

// The size of the VDB window is remembered between sessions.
// This path specifies the path (relative to working directory)
// where the information is stored.
#ifndef VDB_SETTINGS_FILENAME
#define VDB_SETTINGS_FILENAME "./vdb.ini"
#endif

// The state of ImGui windows is remembered between sessions.
// This path specifies the path (relative to working directory)
// where the information is stored.
#ifndef VDB_IMGUI_INI_FILENAME
#define VDB_IMGUI_INI_FILENAME "./imgui.ini"
#endif

// You can set a custom font for ImGui like this:
// #define VDB_FONT "C:/Windows/Fonts/times.ttf", 18.0f

// The maximum number of 'breakpoints' or 'windows that you can step through'
#ifndef VDB_MAX_WINDOWS
#define VDB_MAX_WINDOWS 1024
#endif

#include "so_platform_sdl.h"
#include "imgui.h"


// The viewport is the window region (in pixels from bottom-left) used for drawing.
void vdbViewport(int x, int y, int width, int height);

// The 2D view scales coordinates (i.e. in glVertex) such that x and y are
// mapped from [left, right] and [bottom, top] to the viewport edges.
void vdb2D(float left, float right, float bottom, float top);

// The 3D view gives you an orbital camera that you can control freely.
// You can optionally specify the starting focus point, the field of view,
// and the near and far clipping planes.
void vdb3D(float x=0.0f, float y=0.0f, float z=0.0f, float fov=3.14f/4.0f, float near_clip=0.1f, float far_clip=100.0f);

// Like printf but displays the text at (x,y).
void vdbNote(float x, float y, const char* fmt, ...);

// vdbIsPointHovered and vdbGetHoveredPoint
//   Can be used to select elements using the mouse and conditionally
//   execute code if an element is hovered over. Typical usage is in a for
//   loop drawing a bunch of stuff to the screen, and you want to display
//   information about a specific element, for example, highlighting it
//   and displaying a tooltip about its value.
// EXAMPLE
//   for (int i = 0; i < count; i++)
//   {
//       item = items[i];
//       vdbDrawRect(item.x, item.y, item.w, item.h);
//       if (vdbIsPointHovered(item.center_x, item.center_y))
//       {
//           // Item-specific code here. E.g. display a tooltip
//           SetTooltip("Item index: %d", i);
//       }
//   }
// Returns true if element was hovered over in the previous frame.
bool vdbIsPointHovered(float x, float y, float z = 0.0f, float w = 1.0f);
// Gives the coordinates (x,y,z) of the element that was hovered over
// and returns its index (its place in the coordinates that were passed
// to vdbIsPointHovered).
int vdbGetHoveredPoint(float *x=0, float *y=0, float *z=0);


// VIEWPORT CONVERSIONS
//   NDC (Normalized Device Coordinates) is a space that maps [x=-1,x=+1] to
//   the left and right edges of the viewport, and [y=-1,y=+1] to the bottom
//   and top edges of the viewport. Window coordinates maps x=0 to left, and
//   x=width-1 to right, and y=0 to top and y=height-1 to bottom. The model
//   coordinates are the space that your coordinates in glVertex calls are in.
void vdbNDCToWindow(float x, float y, float *wx, float *wy);
void vdbWindowToNDC(float x, float y, float *nx, float *ny);
void vdbModelToNDC(float x, float y, float z, float w, float *x_ndc, float *y_ndc);
void vdbModelToWindow(float x, float y, float z, float w, float *x_win, float *y_win);


// DRAWING STUFF
void vdbClear(float r, float g, float b, float a);
void vdbFillCircle(float x, float y, float r, int n = 16);
void vdbDrawCircle(float x, float y, float r, int n = 16);
void vdbDrawRect(float x, float y, float w, float h);
void vdbFillRect(float x, float y, float w, float h);
void vdbGridXY(float x_min, float x_max, float y_min, float y_max, int steps);


// COLORS
void vdbColorIndex(int i, float alpha=1.0f); // Calls glColor4f with an indexed color palette
void vdbColorRamp(float t, float alpha=1.0f); // Calls glColor4f with a continuous color ramp
struct vdb_color { float r, g, b, a; };
vdb_color vdbPalette(int i, float alpha=1.0f);
vdb_color vdbPaletteRamp(float t, float alpha=1.0f);
void glColor3f(vdb_color c);
void glColor4f(vdb_color c);
void glClearColor(vdb_color c);
void vdbClear(vdb_color c);


// SHORTHAND FUNCTIONS
void glPoints(float size); // -> glPointSize(size); glBegin(GL_POINTS);
void glLines(float width); // -> glLineWidth(width); glBegin(GL_LINES);
void vdbAdditiveBlend();   // -> glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE);
void vdbAlphaBlend();      // -> glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
void vdbNoBlend();         // -> glDisable(GL_GLEND);


// TEXTURES
//   Can be assigned to a 'slot' which can then be bound to render it on meshes.
//   OpenGL supports many texture formats aside from unsigned 8 bit values, for example,
//   32bit float or 32bit int, as well as anywhere from one to four components (RGBA).
// EXAMPLE
//   unsigned char data[128*128*3];
//   vdbSetTexture(0, data, 128, 128, GL_RGB, GL_UNSIGNED_BYTE);
//   vdbDrawTexture(0);
// EXAMPLE
//                           data_format   data_type
//   Grayscale 32 bit float: GL_LUMINANCE  GL_FLOAT
//   RGB       32 bit float: GL_RGB        GL_FLOAT
//   RGB        8 bit  char: GL_RGB        GL_UNSIGNED_BYTE
void vdbSetTexture(
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
    GLenum internal_format = GL_RGBA);
void vdbDrawTexture(int slot); // Draws the texture to the entire viewport
void vdbBindTexture(int slot); // Enable a texture for custom drawing

// MOUSE BUTTONS
// Down means the button is held down.
// Pressed means it went down this frame.
// Released means it went up this frame.
bool vdbLeftDown();
bool vdbLeftPressed();
bool vdbLeftReleased();
bool vdbRightDown();
bool vdbRightPressed();
bool vdbRightReleased();
bool vdbMiddleDown();
bool vdbMiddlePressed();
bool vdbMiddleReleased();

// KEYBOARD BUTTONS
// Usage: if (vdbKeyCodeDown(SDL_SCANCODE_A)) { ... }.
// Keycodes can be found in SDL_scancode.h.
bool vdbKeyCodeDown(int keycode);
bool vdbKeyCodePressed(int keycode);
bool vdbKeyCodeReleased(int keycode);
// Short-hand functions where you can drop the SDL_SCANCODE_ prefix
// Usage: if (vdbKeyDown(A))
#define vdbKeyDown(KEY)     (vdbKeyCodeDown(SDL_SCANCODE_##KEY))
#define vdbKeyPressed(KEY)  (vdbKeyCodePressed(SDL_SCANCODE_##KEY))
#define vdbKeyReleased(KEY) (vdbKeyCodeReleased(SDL_SCANCODE_##KEY))

// THE VDBB AND VDBE MACROS
// Usage:
// VDBB("Window label");
// {
//     // Code inside here will run at 60 fps
//     // ImGui namespace is automatically included
// }
// VDBE();
//
bool vdb_init(const char *label);
bool vdb_preamble(so_input *input);
void vdb_postamble(so_input input);
void vdb_saveSettings();
#define VDBB(LABEL) if (vdb_init(LABEL)) {                  \
                        so_input vdb_input = {0};           \
                        while (vdb_preamble(&vdb_input)) {  \
                            using namespace ImGui;

#define VDBE()              vdb_postamble(vdb_input);       \
                        }                                   \
                        vdb_saveSettings();                 \
                    }

//
// Implementation starts here
//
#ifndef VDB_HEADER_ONLY
#define SO_PLATFORM_IMPLEMENTATION
#define SO_PLATFORM_IMGUI
#define SO_NOISE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "jo_gif.cpp"
#include "imgui_draw.cpp"
#include "imgui.cpp"
#include "imgui_demo.cpp"
#include "so_platform_sdl.h"
#include <float.h> // FLT_MAX

struct vdb_mat4
{
    union
    {
        float data[16];
        struct
        {
            struct column
            {
                float v1, v2, v3, v4;
            } a1, a2, a3, a4;
        } columns;
    };
    float &operator[](int i) { return data[i]; }
};

struct vdb_vec4
{
    float x, y, z, w;
};

vdb_mat4 vdb_mul4x4(vdb_mat4 a, vdb_mat4 b)
{
    vdb_mat4 c = {0};
    c.columns.a1.v1 = a.columns.a1.v1*b.columns.a1.v1 + a.columns.a2.v1*b.columns.a1.v2 + a.columns.a3.v1*b.columns.a1.v3 + a.columns.a4.v1*b.columns.a1.v4;
    c.columns.a2.v1 = a.columns.a1.v1*b.columns.a2.v1 + a.columns.a2.v1*b.columns.a2.v2 + a.columns.a3.v1*b.columns.a2.v3 + a.columns.a4.v1*b.columns.a2.v4;
    c.columns.a3.v1 = a.columns.a1.v1*b.columns.a3.v1 + a.columns.a2.v1*b.columns.a3.v2 + a.columns.a3.v1*b.columns.a3.v3 + a.columns.a4.v1*b.columns.a3.v4;
    c.columns.a4.v1 = a.columns.a1.v1*b.columns.a4.v1 + a.columns.a2.v1*b.columns.a4.v2 + a.columns.a3.v1*b.columns.a4.v3 + a.columns.a4.v1*b.columns.a4.v4;
    c.columns.a1.v2 = a.columns.a1.v2*b.columns.a1.v1 + a.columns.a2.v2*b.columns.a1.v2 + a.columns.a3.v2*b.columns.a1.v3 + a.columns.a4.v2*b.columns.a1.v4;
    c.columns.a2.v2 = a.columns.a1.v2*b.columns.a2.v1 + a.columns.a2.v2*b.columns.a2.v2 + a.columns.a3.v2*b.columns.a2.v3 + a.columns.a4.v2*b.columns.a2.v4;
    c.columns.a3.v2 = a.columns.a1.v2*b.columns.a3.v1 + a.columns.a2.v2*b.columns.a3.v2 + a.columns.a3.v2*b.columns.a3.v3 + a.columns.a4.v2*b.columns.a3.v4;
    c.columns.a4.v2 = a.columns.a1.v2*b.columns.a4.v1 + a.columns.a2.v2*b.columns.a4.v2 + a.columns.a3.v2*b.columns.a4.v3 + a.columns.a4.v2*b.columns.a4.v4;
    c.columns.a1.v3 = a.columns.a1.v3*b.columns.a1.v1 + a.columns.a2.v3*b.columns.a1.v2 + a.columns.a3.v3*b.columns.a1.v3 + a.columns.a4.v3*b.columns.a1.v4;
    c.columns.a2.v3 = a.columns.a1.v3*b.columns.a2.v1 + a.columns.a2.v3*b.columns.a2.v2 + a.columns.a3.v3*b.columns.a2.v3 + a.columns.a4.v3*b.columns.a2.v4;
    c.columns.a3.v3 = a.columns.a1.v3*b.columns.a3.v1 + a.columns.a2.v3*b.columns.a3.v2 + a.columns.a3.v3*b.columns.a3.v3 + a.columns.a4.v3*b.columns.a3.v4;
    c.columns.a4.v3 = a.columns.a1.v3*b.columns.a4.v1 + a.columns.a2.v3*b.columns.a4.v2 + a.columns.a3.v3*b.columns.a4.v3 + a.columns.a4.v3*b.columns.a4.v4;
    c.columns.a1.v4 = a.columns.a1.v4*b.columns.a1.v1 + a.columns.a2.v4*b.columns.a1.v2 + a.columns.a3.v4*b.columns.a1.v3 + a.columns.a4.v4*b.columns.a1.v4;
    c.columns.a2.v4 = a.columns.a1.v4*b.columns.a2.v1 + a.columns.a2.v4*b.columns.a2.v2 + a.columns.a3.v4*b.columns.a2.v3 + a.columns.a4.v4*b.columns.a2.v4;
    c.columns.a3.v4 = a.columns.a1.v4*b.columns.a3.v1 + a.columns.a2.v4*b.columns.a3.v2 + a.columns.a3.v4*b.columns.a3.v3 + a.columns.a4.v4*b.columns.a3.v4;
    c.columns.a4.v4 = a.columns.a1.v4*b.columns.a4.v1 + a.columns.a2.v4*b.columns.a4.v2 + a.columns.a3.v4*b.columns.a4.v3 + a.columns.a4.v4*b.columns.a4.v4;
    return c;
}

vdb_vec4 vdb_mul4x1(vdb_mat4 a, vdb_vec4 b)
{
    vdb_vec4 c = {0};
    c.x = b.x*a.columns.a1.v1 + b.y*a.columns.a2.v1 + b.z*a.columns.a3.v1 + b.w*a.columns.a4.v1;
    c.y = b.x*a.columns.a1.v2 + b.y*a.columns.a2.v2 + b.z*a.columns.a3.v2 + b.w*a.columns.a4.v2;
    c.z = b.x*a.columns.a1.v3 + b.y*a.columns.a2.v3 + b.z*a.columns.a3.v3 + b.w*a.columns.a4.v3;
    c.w = b.x*a.columns.a1.v4 + b.y*a.columns.a2.v4 + b.z*a.columns.a3.v4 + b.w*a.columns.a4.v4;
    return c;
}

vdb_mat4 vdb_mat_identity()
{
    vdb_mat4 result = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    return result;
}

vdb_mat4 vdb_mat_rot_z(float t)
{
    vdb_mat4 result = { cosf(t),sinf(t),0,0, -sinf(t),cosf(t),0,0, 0,0,1,0, 0,0,0,1 };
    return result;
}

vdb_mat4 vdb_mat_rot_x(float t)
{
    vdb_mat4 result = { 1,0,0,0, 0,cosf(t),sinf(t),0, 0,-sinf(t),cosf(t),0, 0,0,0,1 };
    return result;
}

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

    vdb_mat4 pvm;

    int map_index;
    int map_closest_index;
    int map_prev_closest_index;
    float map_closest_distance;
    float map_closest_x;
    float map_closest_y;
    float map_closest_z;

    int note_index;

    so_input_mouse mouse;
    so_input input;

    const char *window_labels[1024];
    bool        window_hiddens[1024];
    const char *window_label;
    int         window_index;
    int         window_count;

    bool step_once;
    bool step_over;
    bool step_skip;
    bool break_loop;
    bool abort;

    bool first_iteration;
} vdb__globals;

bool vdbLeftDown()       { return !ImGui::GetIO().WantCaptureMouse && vdb__globals.input.left.down; }
bool vdbLeftPressed()    { return !ImGui::GetIO().WantCaptureMouse && vdb__globals.input.left.pressed; }
bool vdbLeftReleased()   { return !ImGui::GetIO().WantCaptureMouse && vdb__globals.input.left.released; }
bool vdbRightDown()      { return !ImGui::GetIO().WantCaptureMouse && vdb__globals.input.right.down; }
bool vdbRightPressed()   { return !ImGui::GetIO().WantCaptureMouse && vdb__globals.input.right.pressed; }
bool vdbRightReleased()  { return !ImGui::GetIO().WantCaptureMouse && vdb__globals.input.right.released; }
bool vdbMiddleDown()     { return !ImGui::GetIO().WantCaptureMouse && vdb__globals.input.middle.down; }
bool vdbMiddlePressed()  { return !ImGui::GetIO().WantCaptureMouse && vdb__globals.input.middle.pressed; }
bool vdbMiddleReleased() { return !ImGui::GetIO().WantCaptureMouse && vdb__globals.input.middle.released; }

bool vdbKeyCodeDown(int keycode)     { return !ImGui::GetIO().WantCaptureKeyboard && vdb__globals.input.keys[keycode].down; }
bool vdbKeyCodePressed(int keycode)  { return !ImGui::GetIO().WantCaptureKeyboard && vdb__globals.input.keys[keycode].pressed; }
bool vdbKeyCodeReleased(int keycode) { return !ImGui::GetIO().WantCaptureKeyboard && vdb__globals.input.keys[keycode].released; }

#define _vdbKeyDown(KEY)     (vdb__globals.input.keys[SO_PLATFORM_KEY(KEY)].down)
#define _vdbKeyPressed(KEY)  (vdb__globals.input.keys[SO_PLATFORM_KEY(KEY)].pressed)
#define _vdbKeyReleased(KEY) (vdb__globals.input.keys[SO_PLATFORM_KEY(KEY)].released)

void vdbStepOnce()
{
    vdb__globals.step_once = true;
}

void vdbViewport(int x, int y, int w, int h)
{
    vdb__globals.viewport_x = x;
    vdb__globals.viewport_y = y;
    vdb__globals.viewport_w = w;
    vdb__globals.viewport_h = h;
    glViewport(x, y, w, h);
}

void vdbNDCToWindow(float x, float y, float *wx, float *wy)
{
    *wx = vdb__globals.viewport_x + (0.5f + 0.5f*x)*vdb__globals.viewport_w;
    *wy = vdb__globals.window_h - 1 - (vdb__globals.viewport_y + (0.5f + 0.5f*y)*vdb__globals.viewport_h);
}

void vdbWindowToNDC(float x, float y, float *nx, float *ny)
{
    *nx = -1.0f + 2.0f * x / vdb__globals.window_w;
    *ny = +1.0f - 2.0f * y / vdb__globals.window_h;
}

void vdbModelToNDC(float x, float y, float z, float w, float *x_ndc, float *y_ndc)
{
    vdb_vec4 model = { x, y, z, w };
    vdb_vec4 clip = vdb_mul4x1(vdb__globals.pvm, model);
    *x_ndc = clip.x / clip.w;
    *y_ndc = clip.y / clip.w;
}

void vdbModelToWindow(float x, float y, float z, float w, float *x_win, float *y_win)
{
    float x_ndc, y_ndc;
    vdbModelToNDC(x, y, z, w, &x_ndc, &y_ndc);
    vdbNDCToWindow(x_ndc, y_ndc, x_win, y_win);
}

void vdbPVM(vdb_mat4 pvm)
{
    vdb__globals.pvm = pvm;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(pvm.data);
}

void vdbPVM(vdb_mat4 projection, vdb_mat4 view, vdb_mat4 model)
{
    vdb__globals.pvm = vdb_mul4x4(vdb_mul4x4(projection, view), model);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(vdb__globals.pvm.data);
}

void vdb2D(float left, float right, float bottom, float top)
{
    float ax = 2.0f/(right-left);
    float ay = 2.0f/(top-bottom);
    float bx = (left+right)/(left-right);
    float by = (bottom+top)/(bottom-top);
    vdb_mat4 projection = {
        ax, 0, 0, 0,
        0, ay, 0, 0,
        0, 0, 1, 0,
        bx, by, 0, 1
    };
    vdbPVM(projection, vdb_mat_identity(), vdb_mat_identity());
}

vdb_mat4 vdb_mat_perspective(float vfov, float aspect, float zn, float zf)
{
    vdb_mat4 projection = {0};
    float t = 1.0f/tanf(vfov/2.0f);
    projection.columns.a1.v1 = t/aspect;
    projection.columns.a2.v2 = t;
    projection.columns.a3.v3 = (zn+zf)/(zn-zf);
    projection.columns.a3.v4 = -1.0f;
    projection.columns.a4.v3 = 2.0f*zn*zf/(zn-zf);
    return projection;
}

vdb_mat4 vdb_mat_sphere(float x, float y, float z, float htheta, float vtheta, float radius)
{
    vdb_mat4 rz = vdb_mat_rot_z(-htheta);
    vdb_mat4 rx = vdb_mat_rot_x(-vtheta);
    vdb_mat4 r = vdb_mul4x4(rx, rz);

    vdb_vec4 f = { -x, -y, -z, 1.0f };
    vdb_vec4 t = vdb_mul4x1(r, f);
    t.z -= radius;

    vdb_mat4 view = r;
    view.columns.a4.v1 += t.x;
    view.columns.a4.v2 += t.y;
    view.columns.a4.v3 += t.z;
    return view;
}

// void vdbView3DOrtho(mat4 view, mat4 model, float h, float z_near, float z_far)
// {
//     float projection[16];
//     projection[vdb_mat1i(4,4,0,0)] =
//     result.a1.x = 2.0f / (right - left);
//     result.a2.y = 2.0f / (top - bottom);
//     result.a3.z = 2.0f / (zn - zf);
//     result.a4.x = (right + left) / (left - right);
//     result.a4.y = (top + bottom) / (bottom - top);
//     result.a4.z = (zf + zn) / (zn - zf);
//     result.a4.w = 1.0f;

//     float w = h*vdb__globals.window_w/vdb__globals.window_h;
//     mat4 projection = mat_ortho_depth(-w/2.0f, +w/2.0f, -h/2.0f, +h/2.0f, z_near, z_far);
//     vdbView(projection, view, model);
// }

void vdb3D(float htheta,
           float vtheta,
           float radius,
           float focus_x,
           float focus_y,
           float focus_z,
           float vfov,
           float zn,
           float zf)
{
    float aspect = vdb__globals.viewport_w/(float)vdb__globals.viewport_h;
    vdb_mat4 p = vdb_mat_perspective(vfov, aspect, zn, zf);
    vdb_mat4 v = vdb_mat_sphere(focus_x, focus_y, focus_z, htheta, vtheta, radius);
    vdb_mat4 m = vdb_mat_identity();
    vdbPVM(p, v, m);
}

void vdb3D(float focus_x0, float focus_y0, float focus_z0, float fov, float zn, float zf)
{
    so_input input = vdb__globals.input;
    static float radius = 1.0f;
    static float htheta = 0.0f;
    static float vtheta = 0.0f;
    static float Rradius = radius;
    static float Rhtheta = htheta;
    static float Rvtheta = vtheta;

    static float focus_x = focus_x0;
    static float focus_y = focus_y0;
    static float focus_z = focus_z0;
    static float Rfocus_x = focus_x;
    static float Rfocus_y = focus_y;
    static float Rfocus_z = focus_z;

    float dt = input.dt;
    if (vdbKeyDown(LSHIFT))
    {
        if (vdbKeyPressed(Z))
            Rradius /= 2.0f;
        if (vdbKeyPressed(X))
            Rradius *= 2.0f;
        if (vdbKeyPressed(LEFT))
            Rhtheta -= 3.1415926f / 4.0f;
        if (vdbKeyPressed(RIGHT))
            Rhtheta += 3.1415926f / 4.0f;
        if (vdbKeyPressed(UP))
            Rvtheta -= 3.1415926f / 4.0f;
        if (vdbKeyPressed(DOWN))
            Rvtheta += 3.1415926f / 4.0f;
        if (vdbKeyPressed(W))
            Rfocus_y += 1.0f;
        if (vdbKeyPressed(S))
            Rfocus_y -= 1.0f;
        if (vdbKeyPressed(A))
            Rfocus_x -= 1.0f;
        if (vdbKeyPressed(D))
            Rfocus_x += 1.0f;
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
        if (vdbKeyDown(W))
            Rfocus_y += dt;
        if (vdbKeyDown(S))
            Rfocus_y -= dt;
        if (vdbKeyDown(A))
            Rfocus_x -= dt;
        if (vdbKeyDown(D))
            Rfocus_x += dt;
    }

    radius += 10.0f * (Rradius - radius) * dt;
    htheta += 10.0f * (Rhtheta - htheta) * dt;
    vtheta += 10.0f * (Rvtheta - vtheta) * dt;
    focus_x += 10.0f * (Rfocus_x - focus_x) * dt;
    focus_y += 10.0f * (Rfocus_y - focus_y) * dt;
    focus_z += 10.0f * (Rfocus_z - focus_z) * dt;

    vdb3D(htheta, vtheta, radius, focus_x, focus_y, focus_z, fov, zn, zf);
}

void vdbNote(float x, float y, const char* fmt, ...)
{
    char name[1024];
    sprintf(name, "vdb_tooltip_%d", vdb__globals.note_index);
    va_list args;
    va_start(args, fmt);

    // Transform position to window coordinates
    float x_win, y_win;
    vdbModelToWindow(x, y, 0.0f, 1.0f, &x_win, &y_win);

    // Clamp tooltip to window
    // (Doesn't work yet)
    #if 0
    {
        char text[1024];
        sprintf(text, fmt, args);

        ImVec2 size = ImGui::CalcTextSize(text);

        if (x_win + size.x + 20.0f > vdb__globals.window_w)
            x_win = vdb__globals.window_w - size.x - 20.0f;

        if (y_win + size.y + 20.0f > vdb__globals.window_h)
            y_win = vdb__globals.window_h - size.y - 20.0f;
    }
    #endif

    ImGui::SetNextWindowPos(ImVec2(x_win, y_win));
    ImGui::Begin(name, 0, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextV(fmt, args);
    ImGui::End();
    va_end(args);
    vdb__globals.note_index++;
}

bool vdbIsPointHovered(float x, float y, float z, float w)
{
    // todo: find closest in z
    float x_win, y_win;
    vdbModelToWindow(x, y, z, w, &x_win, &y_win);

    float dx = x_win - vdb__globals.mouse.x;
    float dy = y_win - vdb__globals.mouse.y;

    float distance = dx*dx + dy*dy;

    if (distance < vdb__globals.map_closest_distance)
    {
        vdb__globals.map_closest_index = vdb__globals.map_index;
        vdb__globals.map_closest_x = x;
        vdb__globals.map_closest_y = y;
        vdb__globals.map_closest_z = z;
        vdb__globals.map_closest_distance = distance;
    }

    bool active_last_frame = vdb__globals.map_index == vdb__globals.map_prev_closest_index;
    vdb__globals.map_index++;
    return active_last_frame;
}

int vdbGetHoveredPoint(float *x, float *y, float *z)
{
    if (x) *x = vdb__globals.map_closest_x;
    if (y) *y = vdb__globals.map_closest_y;
    if (z) *z = vdb__globals.map_closest_z;
    return vdb__globals.map_closest_index;
}

void vdbClear(float r, float g, float b, float a)
{
    vdb_mat4 prev = vdb__globals.pvm;
    vdb2D(-1,+1,-1,+1);
    glBegin(GL_TRIANGLES);
    glColor4f(r, g, b, a);
    glVertex2f(-1,-1);
    glVertex2f(+1,-1);
    glVertex2f(+1,+1);
    glVertex2f(+1,+1);
    glVertex2f(-1,+1);
    glVertex2f(-1,-1);
    glEnd();
    vdbPVM(prev); // restore view
}

void vdbFillCircle(float x, float y, float r, int n)
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

void vdbDrawCircle(float x, float y, float r, int n)
{
    for (int ti = 0; ti < n; ti++)
    {
        float t1 = 2.0f*3.1415926f*(ti+0)/n;
        float t2 = 2.0f*3.1415926f*(ti+1)/n;
        glVertex2f(x+r*cosf(t1), y+r*sinf(t1));
        glVertex2f(x+r*cosf(t2), y+r*sinf(t2));
    }
}

void vdbDrawRect(float x, float y, float w, float h)
{
    glVertex2f(x, y);
    glVertex2f(x+w, y);

    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);

    glVertex2f(x+w, y+h);
    glVertex2f(x, y+h);

    glVertex2f(x, y+h);
    glVertex2f(x, y);
}

void vdbFillRect(float x, float y, float w, float h)
{
    glVertex2f(x, y);
    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);

    glVertex2f(x+w, y+h);
    glVertex2f(x, y+h);
    glVertex2f(x, y);
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


vdb_color vdbPalette(int i, float a)
{
    static vdb_color palette[] =
    {
        { 0.40f, 0.76f, 0.64f, 1.0f },
        { 0.99f, 0.55f, 0.38f, 1.0f },
        { 0.54f, 0.63f, 0.82f, 1.0f },
        { 0.91f, 0.54f, 0.77f, 1.0f },
        { 0.64f, 0.86f, 0.29f, 1.0f },
        { 1.00f, 0.85f, 0.19f, 1.0f },
        { 0.89f, 0.77f, 0.58f, 1.0f },
        { 0.70f, 0.70f, 0.70f, 1.0f }
    };
    const int num_colors = (sizeof(palette) / sizeof((palette)[0]));

    i = i % num_colors;
    vdb_color c = palette[i];
    c.a = a;
    return c;
}

vdb_color vdbPaletteRamp(float t, float a)
{
    float A1 = 0.54f;
    float A2 = 0.55f;
    float A3 = 0.56f;
    float B1 = 0.5f;
    float B2 = 0.5f;
    float B3 = 0.7f;
    float C1 = 0.5f;
    float C2 = 0.5f;
    float C3 = 0.5f;
    float D1 = 0.7f;
    float D2 = 0.8f;
    float D3 = 0.88f;
    float tp = 3.1415926f*2.0f;
    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;
    float r = A1 + B1 * sinf(tp * (C1 * t + D1));
    float g = A2 + B2 * sinf(tp * (C2 * t + D2));
    float b = A3 + B3 * sinf(tp * (C3 * t + D3));
    vdb_color result = { r, g, b, a };
    return result;
}

void vdbColorIndex(int i, float a)
{
    glColor4f(vdbPalette(i, a));
}

void vdbColorRamp(float t, float a)
{
    glColor4f(vdbPaletteRamp(t, a));
}

void glColor3f(vdb_color c) { glColor3f(c.r, c.g, c.b); }
void glColor4f(vdb_color c) { glColor4f(c.r, c.g, c.b, c.a); }
void glClearColor(vdb_color c) { glClearColor(c.r, c.g, c.b, c.a); }
void vdbClear(vdb_color c) { vdbClear(c.r, c.g, c.b, c.a); }

#define vdbSliderFloat(name, val0, val1) static float name = ((val0)+(val1))*0.5f; SliderFloat(#name, &name, val0, val1);
#define vdbSliderInt(name, val0, val1) static int name = val0; SliderInt(#name, &name, val0, val1);
void glPoints(float size) { glPointSize(size); glBegin(GL_POINTS); }
void glLines(float width) { glLineWidth(width); glBegin(GL_LINES); }

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    // glGenerateMipmap(GL_TEXTURE_2D); // todo
    if (min_filter == GL_LINEAR_MIPMAP_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 internal_format,
                 width,
                 height,
                 0,
                 data_format,
                 data_type,
                 data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return result;
}

void vdbSetTexture(
    int slot,
    void *data,
    int width,
    int height,
    GLenum data_format,
    GLenum data_type,
    GLenum mag_filter,
    GLenum min_filter,
    GLenum wrap_s,
    GLenum wrap_t,
    GLenum internal_format)
{
    GLuint tex = 4040 + slot; // Hopefully no one else uses this texture range!
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    // glGenerateMipmap(GL_TEXTURE_2D); // todo
    if (min_filter == GL_LINEAR_MIPMAP_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 internal_format,
                 width,
                 height,
                 0,
                 data_format,
                 data_type,
                 data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void vdbBindTexture(int slot)
{
    glBindTexture(GL_TEXTURE_2D, 4040 + slot);
}

void vdbDrawTexture(int slot)
{
    glEnable(GL_TEXTURE_2D);
    vdbBindTexture(slot);
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

void vdbAdditiveBlend()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
}

void vdbAlphaBlend()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void vdbNoBlend()
{
    glDisable(GL_BLEND);
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

bool vdb_init(const char *label)
{
    static bool have_gui = false;
    if (!have_gui)
    {
        vdb_settings settings = vdb_loadSettings();
        int gl_major = 1;
        int gl_minor = 5;
        so_openWindow("vdb", settings.w, settings.h, settings.x, settings.y, gl_major, gl_minor, VDB_MULTISAMPLES, VDB_ALPHA_BITS, VDB_DEPTH_BITS, VDB_STENCIL_BITS);
        so_imgui_init();
        have_gui = true;
    }

    // Add to window list
    int index = -1;
    {
        for (int i = 0; i < vdb__globals.window_count; i++)
        {
            bool same = true;
            const char *a = vdb__globals.window_labels[i];
            const char *b = label;
            while (*a && *b)
            {
                if (*a != *b)
                {
                    same = false;
                    break;
                }
                a++;
                b++;
            }
            if (same)
                index = i;
        }

        if (index < 0)
        {
            SDL_assert(vdb__globals.window_count+1 <= VDB_MAX_WINDOWS);
            index = vdb__globals.window_count;
            vdb__globals.window_labels[index] = label;
            vdb__globals.window_hiddens[index] = false;
            vdb__globals.window_count++;
        }
    }

    bool is_new = label != vdb__globals.window_label;
    vdb__globals.window_label = label;
    vdb__globals.window_index = index;

    vdb__globals.break_loop = false;
    vdb__globals.step_once = false;
    vdb__globals.step_skip = false;
    vdb__globals.abort = false;

    if (vdb__globals.window_hiddens[index])
        return false; // don't start looping

    if (vdb__globals.step_over && !is_new)
        return false; // don't start looping

    if (vdb__globals.step_over && is_new)
        vdb__globals.step_over = false; // step over is only active while window is the same

    vdb__globals.first_iteration = true;

    return true; // can start looping
}

bool vdb_preamble(so_input *input)
{
    if (!so_loopWindow(input))
        vdb__globals.abort = true;

    if (vdb__globals.break_loop)
        return false;

    if (vdb__globals.abort)
    {
        vdb_saveSettings();
        ImGui::Shutdown();
        exit(1);
    }

    vdb__globals.window_x = input->win_x;
    vdb__globals.window_y = input->win_y;
    vdb__globals.window_w = input->width;
    vdb__globals.window_h = input->height;
    vdb__globals.mouse = input->mouse;
    vdb__globals.input = *input;
    vdb__globals.note_index = 0;

    so_imgui_processEvents(*input);

    vdbViewport(0, 0, input->width, input->height);

    glDisable(GL_DEPTH_TEST);

    glClearColor(0.16f, 0.16f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vdb2D(-1.0f, +1.0f, -1.0f, +1.0f);

    // Specify the start of each pixel row in memory to be 1-byte aligned
    // as opposed to 4-byte aligned or something. Useful to allow for arbitrarily
    // dimensioned textures. Unpack denoting how texture data is _from_ our memory
    // to the GPU.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Reset selection
    {
        vdb__globals.map_closest_distance = FLT_MAX;
        vdb__globals.map_prev_closest_index = vdb__globals.map_closest_index;
        vdb__globals.map_index = 0;
    }

    ImGui::NewFrame();

    return true;
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

void vdb_osd_video_tool(bool *show_video, so_input input)
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

    static bool record_and_step = false;
    static int record_steps = 0;
    static int record_step = 0;

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
        Begin(title, show_video);
    }
    else
    {
        Begin("Record video###Record video", show_video);
    }

    InputText("Filename", format, sizeof(format));
    RadioButton("Animated GIF", &record_mode, record_mode_gif);
    RadioButton("Image sequence", &record_mode, record_mode_img);
    if (record_mode == record_mode_gif)
    {
        InputInt("Frame delay (ms)", &gif_frame_delay);
    }
    Separator();
    InputInt("Frames (0 for no limit)", &frame_limit);
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
    Checkbox("Record and step", &record_and_step);
    if (record_and_step)
    {
        InputInt("Record every...", &record_steps);
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

    bool take_frame = false;
    if (recording)
    {
        if (record_and_step)
        {
            if (record_step % record_steps == 0)
                take_frame = true;
            vdb__globals.step_once = true;
        }
        else
        {
            take_frame = true;
        }
        record_step++;
    }

    if (take_frame)
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

    if (record_mode != record_mode_gif)
    {
        Separator();
        Text("Tip: Use ffmpeg to convert the images to a video file:");
        char *ffmpeg_tip = "ffmpeg -framerate 24 -start_number 1 -i video%04d.png -pix_fmt yuv420p video.mp4";
        InputText("###ffmpeg_tip", ffmpeg_tip, strlen(ffmpeg_tip), ImGuiInputTextFlags_ReadOnly);
    }

    End();

    if (record_region)
    {
        vdb2D(0.0f, (float)input.width, 0.0f, (float)input.height);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
        glVertex2f(region_left+1.0f, region_bottom-1.0f);
        glVertex2f(region_right+1.0f, region_bottom-1.0f);
        glVertex2f(region_right+1.0f, region_top-1.0f);
        glVertex2f(region_left+1.0f, region_top-1.0f);
        glVertex2f(region_left+1.0f, region_bottom-1.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glVertex2f(region_left+0.0f, region_bottom+0.0f);
        glVertex2f(region_right+0.0f, region_bottom+0.0f);
        glVertex2f(region_right+0.0f, region_top+0.0f);
        glVertex2f(region_left+0.0f, region_top+0.0f);
        glVertex2f(region_left+0.0f, region_bottom+0.0f);
        glEnd();
    }
}

#define vdbTriggeredDurationBlock(Event, Duration) \
    static float tdb_timer_##__LINE__ = 0.0f; \
    if (Event) tdb_timer_##__LINE__ = input.t; \
    if (input.t - tdb_timer_##__LINE__ < Duration)

void vdb_postamble(so_input input)
{
    vdb__globals.first_iteration = false;

    vdbViewport(0, 0, input.width, input.height);
    vdb2D(-1.0f, +1.0f, -1.0f, +1.0f);

    using namespace ImGui;

    static bool show_ruler = false;
    static bool show_video = false;
    static bool show_views = false;

    if (_vdbKeyPressed(F4) && _vdbKeyDown(LALT))
    {
        vdb__globals.abort = true;
    }

    bool escape_eaten = false;

    // Ruler
    {
        bool hotkey = _vdbKeyPressed(R) && _vdbKeyDown(LCTRL);
        if (show_ruler && hotkey)
        {
            show_ruler = false;
        }
        else if (!show_ruler && hotkey)
        {
            show_ruler = true;
        }
        if (show_ruler && _vdbKeyPressed(ESCAPE))
        {
            show_ruler = false;
            escape_eaten = true;
        }
    }

    // Video
    {
        bool hotkey = _vdbKeyPressed(V) && _vdbKeyDown(LCTRL);
        if (hotkey)
        {
            show_video = true;
        }
        if (show_video && _vdbKeyPressed(ESCAPE))
        {
            show_video = false;
            escape_eaten = true;
        }
    }

    // Views
    {
        bool hotkey = _vdbKeyPressed(L) & _vdbKeyDown(LCTRL);
        if (hotkey)
        {
            show_views = true;
        }
        if (show_views && _vdbKeyPressed(ESCAPE))
        {
            show_views = false;
            escape_eaten = true;
        }
    }

    // Set window size
    {
        bool hotkey = _vdbKeyPressed(W) && _vdbKeyDown(LCTRL);
        if (hotkey)
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

            if (Button("OK", ImVec2(120,0)) || _vdbKeyPressed(RETURN))
            {
                so_setWindowSize(width, height, topmost);
                CloseCurrentPopup();
            }
            SameLine();
            if (Button("Cancel", ImVec2(120,0)))
            {
                CloseCurrentPopup();
            }
            if (_vdbKeyPressed(ESCAPE))
            {
                CloseCurrentPopup();
                escape_eaten = true;
            }
            EndPopup();
        }
    }

    // Corner protip
    #ifndef VDB_DISABLE_PROTIP
    {
        static float x = -30.0f;
        static float a = 0.4f;
        if (x < 0.0f)
        {
            x += 5.0f*(0.0f-x)*input.dt;
        }
        if (a > 0.1f)
        {
            a += (0.1f-a)*input.dt;
        }
        SetNextWindowPos(ImVec2(x, 0));
        PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
        PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
        PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,a));
        PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0.5f));
        PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,1));
        Begin("##vdb_help_window", 0, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize);
        if (Button("?##vdb_help_button"))
        {
            OpenPopup("Protips##vdb");
        }
        if (BeginPopupModal("Protips##vdb", NULL, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize))
        {
            Text("F10 : Step once");
            Text("F5  : Step over");
            Text("Ctrl+V : Record video");
            Text("Ctrl+R : Show ruler");
            Text("Ctrl+W : Set window size");
            Text("Escape : Close window");
            Text("PrtScr : Take screenshot");
            Text("");
            Text("github.com/lightbits/vdb");
            if (_vdbKeyPressed(ESCAPE) || input.left.pressed)
            {
                CloseCurrentPopup();
                escape_eaten = true;
            }
            EndPopup();
        }
        End();
        PopStyleColor(5);
    }
    #endif

    if (show_ruler)
        vdb_osd_ruler_tool(input);

    if (show_video)
        vdb_osd_video_tool(&show_video, input);

    if (show_views)
    {
        Begin("Hidden views##vdb_windows");
        for (int i = 0; i < vdb__globals.window_count; i++)
            Checkbox(vdb__globals.window_labels[i], &vdb__globals.window_hiddens[i]);
        End();
    }

    // Take screenshot
    {
        if (_vdbKeyReleased(PRINTSCREEN))
        {
            ImGui::OpenPopup("Take screenshot##popup");
        }
        if (BeginPopupModal("Take screenshot##popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char filename[1024];
            vdbTriggeredDurationBlock(_vdbKeyReleased(PRINTSCREEN), 1.0f)
            {
                SetKeyboardFocusHere();
            }
            InputText("Filename", filename, sizeof(filename));

            static bool checkbox;
            Checkbox("32bpp (alpha channel)", &checkbox);

            if (Button("OK", ImVec2(120,0)) || _vdbKeyPressed(RETURN))
            {
                int channels = 3;
                GLenum format = GL_RGB;
                if (checkbox)
                {
                    format = GL_RGBA;
                    channels = 4;
                }
                int width = input.width;
                int height = input.height;
                int stride = width*channels;
                unsigned char *data = (unsigned char*)malloc(height*stride);
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glReadBuffer(GL_BACK);
                glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
                stbi_write_png(filename, width, height, channels, data+stride*(height-1), -stride);
                free(data);
                CloseCurrentPopup();
            }
            SameLine();
            if (Button("Cancel", ImVec2(120,0)))
            {
                CloseCurrentPopup();
            }
            if (_vdbKeyPressed(ESCAPE))
            {
                CloseCurrentPopup();
                escape_eaten = true;
            }
            EndPopup();
        }
    }

    Render();
    so_swapBuffersAndSleep(input.dt);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        assert(false);

    if (_vdbKeyPressed(F10))
    {
        if (_vdbKeyDown(LSHIFT))
            vdb__globals.step_skip = true;
        else
            vdb__globals.step_once = true;
    }
    if (_vdbKeyPressed(F5))
    {
        vdb__globals.step_over = true;
    }
    if (vdb__globals.step_once)
    {
        vdb__globals.break_loop = true;
    }
    if (vdb__globals.step_skip)
    {
        vdb__globals.break_loop = true;
        vdb__globals.window_hiddens[vdb__globals.window_index] = true;
    }
    if (vdb__globals.step_over)
    {
        vdb__globals.break_loop = true;
    }
    if (_vdbKeyPressed(ESCAPE) && !escape_eaten)
    {
        vdb__globals.abort = true;
    }
}

#endif // VDB_HEADER_ONLY
#endif // VDB_HEADER
