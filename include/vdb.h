#pragma once
#include <stdarg.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Types
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef int vdbKey;
typedef int vdbHintKey;
typedef int vdbCameraType;
typedef int vdbOrientation;
typedef int vdbTextureFormat;
typedef int vdbTextureFilter;
typedef int vdbTextureWrap;
struct vdbVec2 { float x,y;     vdbVec2() { x=y=0;     } vdbVec2(float _x, float _y) { x=_x; y=_y; } };
struct vdbVec3 { float x,y,z;   vdbVec3() { x=y=z=0;   } vdbVec3(float _x, float _y, float _z) { x=_x; y=_y; z=_z; } };
struct vdbVec4 { float x,y,z,w; vdbVec4() { x=y=z=w=0; } vdbVec4(float _x, float _y, float _z, float _w) { x=_x; y=_y; z=_z; w=_w; } };
struct vdbRenderTargetSize
{
    int width,height;
    vdbRenderTargetSize(int w, int h) : width(w), height(h) { }
};
struct vdbRenderTargetFormat
{
    vdbTextureFormat format;
    int depth_bits,stencil_bits;
    vdbRenderTargetFormat(vdbTextureFormat f, int d=0, int s=0) : format(f), depth_bits(d), stencil_bits(s) { }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Enums
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern vdbTextureFormat VDB_RGBA32F, VDB_RGBA8;
extern vdbTextureFilter VDB_LINEAR, VDB_LINEAR_MIPMAP, VDB_NEAREST;
extern vdbTextureWrap   VDB_CLAMP, VDB_REPEAT;
extern vdbHintKey       VDB_CAMERA_TYPE;    // value=VDB_PLANAR, etc.
extern vdbHintKey       VDB_ORIENTATION;    // value=VDB_X_DOWN, etc.
extern vdbHintKey       VDB_VIEW_SCALE;     // value=float
extern vdbHintKey       VDB_SHOW_GRID;      // value=bool
extern vdbHintKey       VDB_CAMERA_KEY;     // If set, this key must be pressed to move the camera. value=VDB_KEY_*
extern vdbCameraType    VDB_PLANAR,VDB_TRACKBALL,VDB_TURNTABLE;
extern vdbOrientation   VDB_X_DOWN,VDB_X_UP;
extern vdbOrientation   VDB_Y_DOWN,VDB_Y_UP;
extern vdbOrientation   VDB_Z_DOWN,VDB_Z_UP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Hints:
// These specify initial view settings that are applied on the first rendered
// frame. See vdbHintKey declarations above for a list of available hints.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbHint(vdbHintKey key, int value);
void    vdbHint(vdbHintKey key, float value);
void    vdbHint(vdbHintKey key, bool value);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Immediate mode 2D/3D drawing API
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbInverseColor(bool enable);
void    vdbClearColor(float r, float g, float b, float a);
void    vdbClearDepth(float d);
void    vdbBlendNone();
void    vdbBlendAdd();
void    vdbBlendAlpha();
void    vdbCullFace(bool enable);
void    vdbDepthTest(bool enable);
void    vdbDepthWrite(bool enable);
void    vdbDepthFuncLess();
void    vdbDepthFuncLessOrEqual();
void    vdbDepthFuncAlways();
void    vdbLineWidth(float width);      // line diameter in window units
void    vdbPointSegments(int segments); // points can be rendered as triangles (segments=3), quads (segments=4) or circles or varying fineness (segments > 4)
void    vdbPointSize(float size);       // point diameter in window units
void    vdbPointSize3D(float size);     // point diameter in model coordinates (is affected by projection)
void    vdbBeginLines();
void    vdbBeginPoints();
void    vdbBeginTriangles();
void    vdbEnd();
void    vdbVertex(float x, float y, float z=0.0f, float w=1.0f);
void    vdbColor(float r, float g, float b, float a=1.0f);
void    vdbTexel(float u, float v);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Draw list:
// If you have lots of geometry, you can use vdbBeginList to store the draw
// calls to a static buffer that can be rendered quickly. Typical usage:
//   if (vdbIsFirstFrame()) {
//       vdbBeginList(0);
//       vdbBeginTriangles();
//       ...
//       vdbEnd();
//   }
//   vdbDrawList(0);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbBeginList(int list);
void    vdbDrawList(int list);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Pointer- and array versions of drawing functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbVertex2fv(float *v, float z=0.0f, float w=1.0f);
void    vdbVertex3fv(float *v, float w=1.0f);
void    vdbVertex4fv(float *v);
void    vdbColor4ub (unsigned char r, unsigned char g, unsigned char b, unsigned char a=255);
void    vdbColor4ubv(unsigned char *v);
void    vdbColor3ubv(unsigned char *v, unsigned char a=255);
void    vdbColor4fv (float *v);
void    vdbColor3fv (float *v, float a=1.0f);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Utility drawing functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbNoteV(float x, float y, const char *fmt, va_list args);
void    vdbNote(float x, float y, const char *fmt, ...);
void    vdbFillArc(vdbVec3 base, vdbVec3 p1, vdbVec3 p2, int segments=8);
void    vdbLineCube(float size_x, float size_y, float size_z); // Draws a cube from [-size/2, +size/2] in each axis. Do not call vdbBegin|EndLines before|after.
void    vdbLineCube(vdbVec3 p_min, vdbVec3 p_max);
void    vdbLineGrid(float x_min, float x_max, float y_min, float y_max, int n);
void    vdbLineRect(float x, float y, float size_x, float size_y);
void    vdbFillRect(float x, float y, float size_x, float size_y);
void    vdbLineCircle(float x, float y, float radius, int segments=16);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Matrix stack
// Note: by default vdb interprets matrix pointers as 4x4 column-major arrays.
// You can change this behavior by #defining VDB_ROW_MAJOR before #including
// <vdb.h>. See also section below for the difference between the two.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbPushMatrix();                         // Push matrix stack by one (current top is copied)
void    vdbPopMatrix();                          // Pop matrix stack by one (previous top is restored)
void    vdbProjection(float *m);                 // Pass NULL to load 4x4 identity matrix
void    vdbLoadMatrix(float *m);                 // Pass NULL to load 4x4 identity matrix
void    vdbMultMatrix(float *m);                 // Semantics: M = M mul m (right-multiply top of matrix stack)
void    vdbGetMatrix(float *m);                  // Usage: float m[4*4]; vdbGetMatrix(m);
void    vdbGetProjection(float *m);              // Usage: float m[4*4]; vdbGetProjection(m);
void    vdbGetPVM(float *m);                     // Usage: float m[4*4]; vdbGetPVM(m);
void    vdbTranslate(float x, float y, float z); // Semantics: M <- M mul Translate(x,y,z)
void    vdbRotateXYZ(float x, float y, float z); // Semantics: M <- M mul Rx(x) mul Ry(y) mul Rz(z)
void    vdbRotateZYX(float z, float y, float x); // Semantics: M <- M mul Rz(z) mul Ry(y) mul Rx(x)
void    vdbOrtho(float x_left, float x_right, float y_bottom, float y_top, float z_near=-1.0f, float z_far=+1.0f);
void    vdbPerspective(float yfov, float z_near, float z_far, float x_offset=0.0f, float y_offset=0.0f); // x_offset and y_offset shifts all geometry by a given amount in NDC units (shift is independent of depth)
void    vdbViewporti(int left, int bottom, int width, int height);          // Map all subsequent rendering operations to this region of the window (framebuffer units, not window units)
void    vdbViewport(float left, float bottom, float width, float height);   // Window-size indpendent version of the above (coordinates are in the range [0,1])

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Window information
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float   vdbGetAspectRatio();
int     vdbGetFramebufferWidth();
int     vdbGetFramebufferHeight();
int     vdbGetWindowWidth(); // Note: the window size may not be the same as the framebuffer resolution on retina displays.
int     vdbGetWindowHeight();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Coordinate system conversions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
vdbVec2 vdbModelToNDC   (float x, float y, float z=0.0f, float w=1.0f);
vdbVec3 vdbNDCToModel   (float x, float y, float z=-1.0f);
vdbVec2 vdbWindowToNDC  (float x, float y);
vdbVec2 vdbNDCToWindow  (float x, float y);
vdbVec2 vdbModelToWindow(float x, float y, float z=0.0f, float w=1.0f);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Mouse & Keyboard
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool    vdbWasKeyPressed(vdbKey key);
bool    vdbWasKeyReleased(vdbKey key);
bool    vdbIsKeyDown(vdbKey key);
bool    vdbWasMouseOver(float x, float y, float z=0.0f, float w=1.0f);
int     vdbGetMouseOverIndex(float *x=0, float *y=0, float *z=0);
vdbVec2 vdbGetMousePos();    // upper-left: (0,0). bottom-right: (WindowWidth, WindowHeight)
vdbVec2 vdbGetMousePosNDC(); // bottom-left: (-1,-1). top-right: (+1,+1)
vdbVec3 vdbGetMousePosModel(float depth=-1.0f);
float   vdbGetMouseWheel();
bool    vdbWasMouseLeftPressed();
bool    vdbWasMouseRightPressed();
bool    vdbWasMouseMiddlePressed();
bool    vdbWasMouseLeftReleased();
bool    vdbWasMouseRightReleased();
bool    vdbWasMouseMiddleReleased();
bool    vdbIsMouseLeftDown();
bool    vdbIsMouseRightDown();
bool    vdbIsMouseMiddleDown();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Images
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbLoadImageUint8  (int slot, const void *data, int width, int height, int channels);
void    vdbLoadImageFloat32(int slot, const void *data, int width, int height, int channels);
void    vdbLoadVolumeFloat32(int slot, const void *data, int width, int height, int depth, int channels);
void    vdbDrawImage(int slot, float x, float y, float w, float h, vdbTextureFilter filter=VDB_LINEAR, vdbTextureWrap wrap=VDB_CLAMP, vdbVec4 v_min=vdbVec4(0,0,0,0), vdbVec4 v_max=vdbVec4(1,1,1,1));
void    vdbBindImage(int slot, vdbTextureFilter filter=VDB_LINEAR, vdbTextureWrap wrap=VDB_CLAMP, vdbVec4 v_min=vdbVec4(0,0,0,0), vdbVec4 v_max=vdbVec4(1,1,1,1));
void    vdbUnbindImage();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Shaders
// Your fragment shader must define an entrypoint of the form:
//   void mainImage(out vec4 fragColor, in vec2 fragCoord)
// Your shader has access to these built-in uniform variables:
//   uniform vec2  iResolution;  // Resolution of render target
//   uniform mat4  iPVM;         // Projection*ViewModel matrix
//   uniform mat4  iModelToView; // ViewModel matrix
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbLoadShader(int slot, const char *fragment_shader_source_string);
void    vdbBeginShader(int slot);
void    vdbUniform1f(const char *name, float x);
void    vdbUniform2f(const char *name, float x, float y);
void    vdbUniform3f(const char *name, float x, float y, float z);
void    vdbUniform4f(const char *name, float x, float y, float z, float w);
void    vdbUniform1i(const char *name, int x);
void    vdbUniform2i(const char *name, int x, int y);
void    vdbUniform3i(const char *name, int x, int y, int z);
void    vdbUniform4i(const char *name, int x, int y, int z, int w);
void    vdbUniformMatrix4fv(const char *name, float *x);
void    vdbUniformMatrix3fv(const char *name, float *x);
void    vdbUniformSampler(const char *name, int slot, int sampler_offset=0, vdbTextureFilter filter=VDB_LINEAR, vdbTextureWrap wrap=VDB_CLAMP);
void    vdbEndShader();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Render targets
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbBeginRenderTarget(int slot, vdbRenderTargetSize size, vdbRenderTargetFormat format);
void    vdbEndRenderTarget(int slot);
void    vdbBindRenderTarget(int slot, vdbTextureFilter filter=VDB_LINEAR, vdbTextureWrap wrap=VDB_CLAMP);
void    vdbDrawRenderTarget(int slot, vdbTextureFilter filter=VDB_LINEAR, vdbTextureWrap wrap=VDB_CLAMP);
void    vdbUnbindRenderTarget();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Widgets
// Note: vdb includes Dear ImGui (https://github.com/ocornut/imgui), which can
// be accessed by #include <vdb/imgui.h> (see test/test.cpp).
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float   vdbSliderFloat(const char *name, float vmin, float vmax, float v_init);
int     vdbSliderInt  (const char *name, int vmin, int vmax, int v_init);
bool    vdbCheckbox   (const char *name, bool init);
bool    vdbRadioButton(const char *name);
bool    vdbButton     (const char *name);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Render scaler
// This lets you render at a lower resolution. You can also generate a high-
// resolution frame by interleaving several low-resolution frames. This is
// done using an 'up' factor greater than 0. An up factor of 1 increases the
// resolution by 4x (2x in each dimension) by interleaving four frames.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbBeginRenderScale(int down, int up);
void    vdbBeginRenderScale(int width, int height, int up);
void    vdbEndRenderScale();
vdbVec2 vdbGetRenderScale();
vdbVec2 vdbGetRenderOffset();
vdbVec2 vdbGetRenderOffsetFramebuffer();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Low-level functionality
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbMakeContextCurrent();
void    vdbDetachContext();
void    vdbStepOnce();
void    vdbStepOver();
bool    vdbBeginBreak(const char *label);
void    vdbEndBreak();
bool    vdbIsFirstFrame();
bool    vdbIsDifferentLabel();
void    vdbAutoStep(bool enabled);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Logging
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbLogPush(const char *label);
void    vdbLogPush();
void    vdbLogPop();
void    vdbLogScalar(const char *label, float x);
void    vdbLogMatrix(const char *label, float *x, int rows, int columns);
void    vdbLogVector(const char *label, float *x, int elements);
void    vdbLogDump(const char *filename);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Row-major versions of matrix functions:
// Row-major assumes matrix elements are laid out in memory one row at a time.
// By default, vdb functions accepting matrices assume column-major.
//
// Row-major:
//   float m[] = {A,B,C,D}; -> |A B|
//                             |C D|
//
// Column-major:
//   float m[] = {A,B,C,D}; -> |A C|
//                             |B D|
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbProjection_RowMaj(float *m);
void    vdbLoadMatrix_RowMaj(float *m);
void    vdbMultMatrix_RowMaj(float *m);
void    vdbGetMatrix_RowMaj(float *m);
void    vdbGetProjection_RowMaj(float *m);
void    vdbGetPVM_RowMaj(float *m);
void    vdbUniformMatrix4fv_RowMaj(const char *name, float *x);
void    vdbUniformMatrix3fv_RowMaj(const char *name, float *x);
void    vdbLogMatrix_RowMaj(const char *label, float *x, int rows, int columns);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Redefine row-major matrix as default
// This simply redefines the some vdb functions to use the row-major version.
// To enable: #define VDB_ROW_MAJOR before including vdb.h
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef VDB_ROW_MAJOR
#define vdbProjection       vdbProjection_RowMaj
#define vdbLoadMatrix       vdbLoadMatrix_RowMaj
#define vdbMultMatrix       vdbMultMatrix_RowMaj
#define vdbGetMatrix        vdbGetMatrix_RowMaj
#define vdbGetProjection    vdbGetProjection_RowMaj
#define vdbGetPVM           vdbGetPVM_RowMaj
#define vdbUniformMatrix4fv vdbUniformMatrix4fv_RowMaj
#define vdbUniformMatrix3fv vdbUniformMatrix3fv_RowMaj
#define vdbLogMatrix        vdbLogMatrix_RowMaj
#endif

#define VDBB(label) while (vdbBeginBreak(label)) {
#define VDBE() vdbEndBreak(); }

extern vdbKey VDB_KEY_A;
extern vdbKey VDB_KEY_B;
extern vdbKey VDB_KEY_C;
extern vdbKey VDB_KEY_D;
extern vdbKey VDB_KEY_E;
extern vdbKey VDB_KEY_F;
extern vdbKey VDB_KEY_G;
extern vdbKey VDB_KEY_H;
extern vdbKey VDB_KEY_I;
extern vdbKey VDB_KEY_J;
extern vdbKey VDB_KEY_K;
extern vdbKey VDB_KEY_L;
extern vdbKey VDB_KEY_M;
extern vdbKey VDB_KEY_N;
extern vdbKey VDB_KEY_O;
extern vdbKey VDB_KEY_P;
extern vdbKey VDB_KEY_Q;
extern vdbKey VDB_KEY_R;
extern vdbKey VDB_KEY_S;
extern vdbKey VDB_KEY_T;
extern vdbKey VDB_KEY_U;
extern vdbKey VDB_KEY_V;
extern vdbKey VDB_KEY_W;
extern vdbKey VDB_KEY_X;
extern vdbKey VDB_KEY_Y;
extern vdbKey VDB_KEY_Z;

extern vdbKey VDB_KEY_1;
extern vdbKey VDB_KEY_2;
extern vdbKey VDB_KEY_3;
extern vdbKey VDB_KEY_4;
extern vdbKey VDB_KEY_5;
extern vdbKey VDB_KEY_6;
extern vdbKey VDB_KEY_7;
extern vdbKey VDB_KEY_8;
extern vdbKey VDB_KEY_9;
extern vdbKey VDB_KEY_0;

extern vdbKey VDB_KEY_RETURN;
extern vdbKey VDB_KEY_ESCAPE;
extern vdbKey VDB_KEY_BACKSPACE;
extern vdbKey VDB_KEY_TAB;
extern vdbKey VDB_KEY_SPACE;

extern vdbKey VDB_KEY_F1;
extern vdbKey VDB_KEY_F2;
extern vdbKey VDB_KEY_F3;
extern vdbKey VDB_KEY_F4;
extern vdbKey VDB_KEY_F5;
extern vdbKey VDB_KEY_F6;
extern vdbKey VDB_KEY_F7;
extern vdbKey VDB_KEY_F8;
extern vdbKey VDB_KEY_F9;
extern vdbKey VDB_KEY_F10;
extern vdbKey VDB_KEY_F11;
extern vdbKey VDB_KEY_F12;

extern vdbKey VDB_KEY_HOME;
extern vdbKey VDB_KEY_PAGEUP;
extern vdbKey VDB_KEY_DELETE;
extern vdbKey VDB_KEY_END;
extern vdbKey VDB_KEY_PAGEDOWN;
extern vdbKey VDB_KEY_RIGHT;
extern vdbKey VDB_KEY_LEFT;
extern vdbKey VDB_KEY_DOWN;
extern vdbKey VDB_KEY_UP;

extern vdbKey VDB_KEY_LCTRL;
extern vdbKey VDB_KEY_LSHIFT;
extern vdbKey VDB_KEY_LALT; /**< alt, option */
extern vdbKey VDB_KEY_LGUI; /**< windows, command (apple), meta */

extern vdbKey VDB_KEY_RCTRL;
extern vdbKey VDB_KEY_RSHIFT;
extern vdbKey VDB_KEY_RALT; /**< alt gr, option */
extern vdbKey VDB_KEY_RGUI; /**< windows, command (apple), meta */

enum { VDB_NUM_KEYS = 512 };
