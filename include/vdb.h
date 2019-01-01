#pragma once
#include <stdarg.h>

struct vdbVec2 { float x,y;     vdbVec2() { x=y=0;     } vdbVec2(float _x, float _y) { x=_x; y=_y; } };
struct vdbVec3 { float x,y,z;   vdbVec3() { x=y=z=0;   } vdbVec3(float _x, float _y, float _z) { x=_x; y=_y; z=_z; } };
struct vdbVec4 { float x,y,z,w; vdbVec4() { x=y=z=w=0; } vdbVec4(float _x, float _y, float _z, float _w) { x=_x; y=_y; z=_z; w=_w; } };

// Enumerations (defined as int to be compatible with some c++ compilers)
typedef int vdbKey;
typedef int vdbTextureFormat;
typedef int vdbTextureFilter;
typedef int vdbTextureWrap;

// vdb.cpp
void vdbDetachGLContext();
bool vdbBeginFrame(const char *label);
void vdbEndFrame();
bool vdbIsFirstFrame();

// vdb_immediate.cpp
void vdbInverseColor(bool enable);
void vdbClearColor(float r, float g, float b, float a=1.0f);
void vdbClearDepth(float d);
void vdbBlendNone();
void vdbBlendAdd();
void vdbBlendAlpha();
void vdbCullFace(bool enable);       // if enabled, back-facing triangles are not drawn
void vdbDepthTest(bool enable);      // if enabled, fragments are tested against depth buffer
void vdbDepthWrite(bool enable);     // if enabled, fragments write to the depth buffer
void vdbLineWidth(float width);      // line diameter in framebuffer pixels
void vdbPointSegments(int segments); // points can be rendered as triangles (segments=3), quads (segments=4) or circles or varying fineness (segments > 4)
void vdbPointSize(float size);       // point diameter in framebuffer pixels (size=1 and segments=4 gives pixel-perfect rendering)
void vdbPointSize3D(float size);     // point diameter in model-coordinates (is affected by projection)
void vdbBeginLines();
void vdbBeginPoints();
void vdbTriangles();
void vdbLines(float width); // short-hand for vdbLineWidth(width); vdbBeginLines();
void vdbPoints(float size); // short-hand for vdbPointSize(size); vdbBeginPoints();
void vdbEnd();
void vdbBeginList(int list); // call before vdbBegin* to store the resulting geometry stream to a buffer (you can draw the buffer by calling vdbDrawList)
void vdbDrawList(int list);
void vdbVertex(float x, float y, float z=0.0f, float w=1.0f);
void vdbVertex2fv(float *v, float z=0.0f, float w=1.0f);
void vdbVertex3fv(float *v, float w=1.0f);
void vdbVertex4fv(float *v);
void vdbVertex(vdbVec3 xyz, float w=1.0f);
void vdbVertex(vdbVec4 xyzw);
void vdbColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void vdbColor4ubv(unsigned char *v);
void vdbColor3ubv(unsigned char *v, unsigned char a=255);
void vdbColor(float r, float g, float b, float a=1.0f);
void vdbColor4fv(float *v);
void vdbColor3fv(float *v, float a=1.0f);
void vdbColor(vdbVec3 rgb, float a=1.0f);
void vdbColor(vdbVec4 rgba);
void vdbTexel(float u, float v);

// vdb_immediate_util.cpp
void vdbNoteV(float x, float y, const char *fmt, va_list args);
void vdbNote(float x, float y, const char *fmt, ...);
void vdbFillArc(vdbVec3 base, vdbVec3 p1, vdbVec3 p2, int segments=8);
void vdbLineCube(float size_x, float size_y, float size_z); // Draws a cube from [-size/2, +size/2] in each axis. Do not call vdbBegin|EndLines before|after.
void vdbLineGrid(float x_min, float x_max, float y_min, float y_max, int n);
void vdbLineRect(float x, float y, float size_x, float size_y);
void vdbFillRect(float x, float y, float size_x, float size_y);
void vdbLineCircle(float x, float y, float radius, int segments=16);

// vdb_transform.cpp
// VDB can be compiled to accept matrices in either row- or column-major memory order. See vdb_config.h
// Matrix = transformation from model coordinates to view coordinates (before projection)
void vdbPushMatrix();                         // Push matrix stack by one (current top is copied)
void vdbPopMatrix();                          // Pop matrix stack by one (previous top is restored)
void vdbProjection(float *m);                 // NULL -> Load 4x4 identity matrix
void vdbLoadMatrix(float *m);                 // NULL -> Load 4x4 identity matrix
void vdbMultMatrix(float *m);                 // Matrix <- Matrix mul m (right-multiply top of matrix stack)
void vdbGetMatrix(float *m);                  // You allocate m, e.g.: float matrix[4*4]; vdbGetMatrix(matrix);
void vdbGetProjection(float *m);              // You allocate m, e.g.: float projection[4*4]; vdbGetProjection(projection);
void vdbGetPVM(float *m);                     // You allocate m, e.g.: float pvm[4*4]; vdbGetPVM(pvm);
void vdbTranslate(float x, float y, float z); // Matrix <- Matrix mul Translate(x,y,z)
void vdbRotateXYZ(float x, float y, float z); // Matrix <- Matrix mul Rx(x) mul Ry(y) mul Rz(z)
void vdbRotateZYX(float z, float y, float x); // Matrix <- Matrix mul Rz(z) mul Ry(y) mul Rx(x)
void vdbViewporti(int left, int bottom, int width, int height);          // Map all subsequent rendering operations to this region of the window (framebuffer units, not window units)
void vdbViewport(float left, float bottom, float width, float height);   // Window-size indpendent version of the above (coordinates are in the range [0,1])
void vdbOrtho(float x_left, float x_right, float y_bottom, float y_top);
void vdbOrtho(float x_left, float x_right, float y_bottom, float y_top, float z_near, float z_far);
void vdbPerspective(float yfov, float z_near, float z_far, float x_offset=0.0f, float y_offset=0.0f); // x_offset and y_offset shifts all geometry by a given amount in NDC units (shift is independent of depth)

// vdb_transform.cpp
vdbVec2 vdbNDCToWindow(float xn, float yn);
vdbVec2 vdbWindowToNDC(float xw, float yw);
vdbVec3 vdbNDCToModel(float x_ndc, float y_ndc, float depth=-1.0f);
vdbVec2 vdbModelToNDC(float x, float y, float z=0.0f, float w=1.0f);
float vdbGetAspectRatio();
int vdbGetFramebufferWidth();
int vdbGetFramebufferHeight();
int vdbGetWindowWidth(); // Note: the window size may not be the same as the framebuffer resolution on retina displays.
int vdbGetWindowHeight();

// vdb_camera.cpp
void vdbCameraTrackball();
void vdbCameraTurntable(float init_radius=1.0f, vdbVec3 look_at=vdbVec3());

// vdb_keyboard.cpp
bool vdbWasKeyPressed(vdbKey key);
bool vdbWasKeyReleased(vdbKey key);
bool vdbIsKeyDown(vdbKey key);

// vdb_mouse.cpp
bool vdbWasMouseOver(float x, float y, float z=0.0f, float w=1.0f);
int vdbGetMouseOverIndex(float *x=0, float *y=0, float *z=0);
vdbVec2 vdbGetMousePos();    // upper-left: (0,0). bottom-right: (WindowWidth, WindowHeight)
vdbVec2 vdbGetMousePosNDC(); // bottom-left: (-1,-1). top-right: (+1,+1)
vdbVec3 vdbGetMousePosModel(float depth=-1.0f);
float vdbGetMouseWheel();
bool vdbWasMouseLeftPressed();
bool vdbWasMouseRightPressed();
bool vdbWasMouseMiddlePressed();
bool vdbWasMouseLeftReleased();
bool vdbWasMouseRightReleased();
bool vdbWasMouseMiddleReleased();
bool vdbIsMouseLeftDown();
bool vdbIsMouseRightDown();
bool vdbIsMouseMiddleDown();

// vdb_image.cpp
void vdbLoadImageUint8(int slot, const void *data, int width, int height, int channels);
void vdbLoadImageFloat32(int slot, const void *data, int width, int height, int channels);
void vdbLoadImageFromFile(int slot, const char *filename, int *width=0, int *height=0, int *channels=0);
void vdbDrawImage(int slot);
void vdbBindImage(int slot);
void vdbUnbindImage();

// vdb_filter.cpp
void vdbBeginTAA(int downscale, float blend_factor);
void vdbBeginTSS(int width, int height, int upscale, float *dx, float *dy);
void vdbEndTAA();
void vdbEndTSS();

// vdb_shader.cpp
void vdbLoadShader(int slot, const char *fragment_shader_source_string);
void vdbBeginShader(int slot);
void vdbEndShader();
void vdbUniform1f(const char *name, float x);
void vdbUniform2f(const char *name, float x, float y);
void vdbUniform3f(const char *name, float x, float y, float z);
void vdbUniform4f(const char *name, float x, float y, float z, float w);
void vdbUniform1i(const char *name, int x);
void vdbUniform2i(const char *name, int x, int y);
void vdbUniform3i(const char *name, int x, int y, int z);
void vdbUniform4i(const char *name, int x, int y, int z, int w);
void vdbUniformMatrix4fv(const char *name, float *x, bool transpose=false);
void vdbUniformMatrix3fv(const char *name, float *x, bool transpose=false);

// vdb_render_texture.cpp
void vdbBeginRenderTexture(int slot, int width, int height, vdbTextureFormat format, int depth_bits=0, int stencil_bits=0);
void vdbEndRenderTexture(int slot);
void vdbBindRenderTexture(int slot);
void vdbUnbindRenderTexture();
void vdbDrawRenderTexture(int slot);

void vdbSetTextureParameters(vdbTextureFilter filter, vdbTextureWrap wrap, bool generate_mipmaps); // applies to the currently bound texture (e.g. from BindRenderTexture or BindImage)

#define VDBB(label) while (vdbBeginFrame(label)) {
#define VDBE() vdbEndFrame(); }

// Subset of scancodes copied from SDL_scancodes.h
enum vdbKey_
{
    VDB_KEY_A = 4,
    VDB_KEY_B = 5,
    VDB_KEY_C = 6,
    VDB_KEY_D = 7,
    VDB_KEY_E = 8,
    VDB_KEY_F = 9,
    VDB_KEY_G = 10,
    VDB_KEY_H = 11,
    VDB_KEY_I = 12,
    VDB_KEY_J = 13,
    VDB_KEY_K = 14,
    VDB_KEY_L = 15,
    VDB_KEY_M = 16,
    VDB_KEY_N = 17,
    VDB_KEY_O = 18,
    VDB_KEY_P = 19,
    VDB_KEY_Q = 20,
    VDB_KEY_R = 21,
    VDB_KEY_S = 22,
    VDB_KEY_T = 23,
    VDB_KEY_U = 24,
    VDB_KEY_V = 25,
    VDB_KEY_W = 26,
    VDB_KEY_X = 27,
    VDB_KEY_Y = 28,
    VDB_KEY_Z = 29,

    VDB_KEY_1 = 30,
    VDB_KEY_2 = 31,
    VDB_KEY_3 = 32,
    VDB_KEY_4 = 33,
    VDB_KEY_5 = 34,
    VDB_KEY_6 = 35,
    VDB_KEY_7 = 36,
    VDB_KEY_8 = 37,
    VDB_KEY_9 = 38,
    VDB_KEY_0 = 39,

    VDB_KEY_RETURN = 40,
    VDB_KEY_ESCAPE = 41,
    VDB_KEY_BACKSPACE = 42,
    VDB_KEY_TAB = 43,
    VDB_KEY_SPACE = 44,

    VDB_KEY_F1 = 58,
    VDB_KEY_F2 = 59,
    VDB_KEY_F3 = 60,
    VDB_KEY_F4 = 61,
    VDB_KEY_F5 = 62,
    VDB_KEY_F6 = 63,
    VDB_KEY_F7 = 64,
    VDB_KEY_F8 = 65,
    VDB_KEY_F9 = 66,
    VDB_KEY_F10 = 67,
    VDB_KEY_F11 = 68,
    VDB_KEY_F12 = 69,

    VDB_KEY_HOME = 74,
    VDB_KEY_PAGEUP = 75,
    VDB_KEY_DELETE = 76,
    VDB_KEY_END = 77,
    VDB_KEY_PAGEDOWN = 78,
    VDB_KEY_RIGHT = 79,
    VDB_KEY_LEFT = 80,
    VDB_KEY_DOWN = 81,
    VDB_KEY_UP = 82,

    VDB_KEY_LCTRL = 224,
    VDB_KEY_LSHIFT = 225,
    VDB_KEY_LALT = 226, /**< alt, option */
    VDB_KEY_LGUI = 227, /**< windows, command (apple), meta */

    VDB_KEY_RCTRL = 228,
    VDB_KEY_RSHIFT = 229,
    VDB_KEY_RALT = 230, /**< alt gr, option */
    VDB_KEY_RGUI = 231, /**< windows, command (apple), meta */
};

enum vdbTextureFormat_ { VDB_RGBA32F, VDB_RGBA8U };
enum vdbTextureFilter_ { VDB_LINEAR, VDB_LINEAR_MIPMAP, VDB_NEAREST };
enum vdbTextureWrap_ { VDB_CLAMP, VDB_REPEAT };
