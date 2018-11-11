#pragma once
#include <stdarg.h>

struct vdbVec2 { float x,y;     vdbVec2() { x=y=0;     } vdbVec2(float _x, float _y) { x=_x; y=_y; } };
struct vdbVec3 { float x,y,z;   vdbVec3() { x=y=z=0;   } vdbVec3(float _x, float _y, float _z) { x=_x; y=_y; z=_z; } };
struct vdbVec4 { float x,y,z,w; vdbVec4() { x=y=z=w=0; } vdbVec4(float _x, float _y, float _z, float _w) { x=_x; y=_y; z=_z; w=_w; } };
enum vdbKey;

void vdbDetachGLContext();
bool vdbBeginFrame(const char *label);
void vdbEndFrame();
bool vdbIsFirstFrame();
void vdbNoteV(float x, float y, const char *fmt, va_list args);
void vdbNote(float x, float y, const char *fmt, ...);
void vdbClearColor(float r, float g, float b, float a=1.0f);
void vdbClearDepth(float d);
void vdbLineWidth(float width);
void vdbBeginLines();
void vdbLines(float width);
void vdbLineRect(float x, float y, float w, float h);
void vdbPointSize(float radius);
void vdbBeginPoints();
void vdbPoints(float radius);
void vdbTriangles();
void vdbEnd();
void vdbVertex(float x, float y, float z=0.0f, float w=1.0f);
void vdbColor(float r, float g, float b, float a=1.0f);
void vdbTexel(float u, float v);
void vdbInverseColor(bool enable);
void vdbBlendNone();
void vdbBlendAdd();
void vdbBlendAlpha();
void vdbDepthTest(bool enable);
void vdbDepthWrite(bool enable);
void vdbProjection(float *m=0);
void vdbPushMatrix(float *m=0);
void vdbPopMatrix();
void vdbMatrix(float *m);
void vdbGetMatrix(float *m);
void vdbMatrixEulerXYZ(float tx,float ty,float tz, float rx,float ry,float rz);
void vdbMatrixEulerZYX(float tx,float ty,float tz, float rz,float ry,float rx);
void vdbPushMatrixEulerXYZ(float tx,float ty,float tz, float rx,float ry,float rz);
void vdbPushMatrixEulerZYX(float tx,float ty,float tz, float rz,float ry,float rx);
void vdbViewport(float left, float bottom, float width, float height);
void vdbOrtho(float x_left, float x_right, float y_bottom, float y_top);
void vdbOrtho(float x_left, float x_right, float y_bottom, float y_top, float z_near, float z_far);
void vdbPerspective(float yfov, float width, float height, float z_near, float z_far, float x_offset=0.0f, float y_offset=0.0f);
vdbVec2 vdbNDCToWindow(float xn, float yn);
vdbVec2 vdbWindowToNDC(float xw, float yw);
vdbVec3 vdbNDCToModel(float x_ndc, float y_ndc, float depth=-1.0f);
vdbVec2 vdbModelToNDC(float x, float y, float z=0.0f, float w=1.0f);
int vdbGetFramebufferWidth();
int vdbGetFramebufferHeight();
float vdbGetAspectRatio();
int vdbGetWindowWidth();
int vdbGetWindowHeight();
bool vdbIsMouseOver(float x, float y, float z=0.0f, float w=1.0f);
int vdbGetMouseOverIndex(float *x=0, float *y=0, float *z=0);
vdbVec2 vdbGetMousePos();
vdbVec2 vdbGetMousePosNDC();
vdbVec3 vdbGetMousePosModel(float depth=-1.0f);
bool vdbIsKeyPressed(vdbKey key);
bool vdbIsKeyDown(vdbKey key);
bool vdbIsKeyReleased(vdbKey key);
bool vdbIsMouseLeftPressed();
bool vdbIsMouseLeftDown();
bool vdbIsMouseLeftReleased();
bool vdbIsMouseRightPressed();
bool vdbIsMouseRightDown();
bool vdbIsMouseRightReleased();
bool vdbIsMouseMiddlePressed();
bool vdbIsMouseMiddleDown();
bool vdbIsMouseMiddleReleased();
float vdbGetMouseWheel();
void vdbLoadPoints(int slot, vdbVec3 *position, vdbVec4 *color, int num_points);
void vdbDrawPoints(int slot, float point_size, int circle_segments);
void vdbLoadImageUint8(int slot, const void *data, int width, int height, int channels);
void vdbLoadImageFloat32(int slot, const void *data, int width, int height, int channels);
void vdbLoadImageFromFile(int slot, const char *filename, int *width=0, int *height=0, int *channels=0);
void vdbDrawImage(int slot);
void vdbBindImage(int slot);
void vdbUnbindImage();
void vdbBeginTAA(int downscale, float blend_factor);
void vdbBeginTSS(int width, int height, int upscale, float *dx, float *dy);
void vdbEndTAA();
void vdbEndTSS();
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

// Subset of scancodes copied from SDL_scancodes.h
enum vdbKey
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

#define VDBB(label) while (vdbBeginFrame(label)) {
#define VDBE() vdbEndFrame(); }
