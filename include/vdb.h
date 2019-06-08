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
// These specify initial view settings that are applied
// on the first rendered frame. E.g. camera orientation,
// camera type, view scale.
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
void    vdbLineWidth(float width);      // line diameter in framebuffer pixels
void    vdbPointSegments(int segments); // points can be rendered as triangles (segments=3), quads (segments=4) or circles or varying fineness (segments > 4)
void    vdbPointSize(float size);       // point diameter in framebuffer pixels (size=1 and segments=4 gives pixel-perfect rendering)
void    vdbPointSize3D(float size);     // point diameter in model-coordinates (is affected by projection)
void    vdbBeginLines();
void    vdbBeginPoints();
void    vdbBeginTriangles();
void    vdbEnd();
void    vdbVertex(float x, float y, float z=0.0f, float w=1.0f);
void    vdbColor(float r, float g, float b, float a=1.0f);
void    vdbTexel(float u, float v);
void    vdbBeginList(int list); // call before vdbBegin* to store the resulting geometry stream to a buffer (you can draw the buffer by calling vdbDrawList)
void    vdbDrawList(int list);

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
void    vdbVertex2fv(float *v, float z=0.0f, float w=1.0f);
void    vdbVertex3fv(float *v, float w=1.0f);
void    vdbVertex4fv(float *v);
void    vdbColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void    vdbColor4ubv(unsigned char *v);
void    vdbColor3ubv(unsigned char *v, unsigned char a=255);
void    vdbColor4fv(float *v);
void    vdbColor3fv(float *v, float a=1.0f);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Matrix stack
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbPushMatrix();                         // Push matrix stack by one (current top is copied)
void    vdbPopMatrix();                          // Pop matrix stack by one (previous top is restored)
void    vdbProjection(float *m);                 // NULL -> Load 4x4 identity matrix
void    vdbLoadMatrix(float *m);                 // NULL -> Load 4x4 identity matrix
void    vdbMultMatrix(float *m);                 // Matrix <- Matrix mul m (right-multiply top of matrix stack)
void    vdbGetMatrix(float *m);                  // You allocate m, e.g.: float matrix[4*4]; vdbGetMatrix(matrix);
void    vdbGetProjection(float *m);              // You allocate m, e.g.: float projection[4*4]; vdbGetProjection(projection);
void    vdbGetPVM(float *m);                     // You allocate m, e.g.: float pvm[4*4]; vdbGetPVM(pvm);
void    vdbTranslate(float x, float y, float z); // Matrix <- Matrix mul Translate(x,y,z)
void    vdbRotateXYZ(float x, float y, float z); // Matrix <- Matrix mul Rx(x) mul Ry(y) mul Rz(z)
void    vdbRotateZYX(float z, float y, float x); // Matrix <- Matrix mul Rz(z) mul Ry(y) mul Rx(x)
void    vdbOrtho(float x_left, float x_right, float y_bottom, float y_top, float z_near=-1.0f, float z_far=+1.0f);
void    vdbPerspective(float yfov, float z_near, float z_far, float x_offset=0.0f, float y_offset=0.0f); // x_offset and y_offset shifts all geometry by a given amount in NDC units (shift is independent of depth)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Window
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbViewporti(int left, int bottom, int width, int height);          // Map all subsequent rendering operations to this region of the window (framebuffer units, not window units)
void    vdbViewport(float left, float bottom, float width, float height);   // Window-size indpendent version of the above (coordinates are in the range [0,1])
vdbVec2 vdbNDCToWindow(float xn, float yn);
vdbVec2 vdbWindowToNDC(float xw, float yw);
vdbVec3 vdbNDCToModel(float x_ndc, float y_ndc, float depth=-1.0f);
vdbVec2 vdbModelToNDC(float x, float y, float z=0.0f, float w=1.0f);
vdbVec2 vdbModelToWindow(float x, float y, float z=0.0f, float w=1.0f);
float   vdbGetAspectRatio();
int     vdbGetFramebufferWidth();
int     vdbGetFramebufferHeight();
int     vdbGetWindowWidth(); // Note: the window size may not be the same as the framebuffer resolution on retina displays.
int     vdbGetWindowHeight();

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
void    vdbBindImage(int slot, vdbTextureFilter filter=VDB_LINEAR, vdbTextureWrap wrap=VDB_CLAMP, vdbVec4 v_min=vdbVec4(0,0,0,0), vdbVec4 v_max=vdbVec4(1,1,1,1));
void    vdbUnbindImage();
void    vdbLoadImageUint8(int slot, const void *data, int width, int height, int channels);
void    vdbLoadImageFloat32(int slot, const void *data, int width, int height, int channels);
void    vdbDrawImage(int slot, float x, float y, float w, float h, vdbTextureFilter filter=VDB_LINEAR, vdbTextureWrap wrap=VDB_CLAMP, vdbVec4 v_min=vdbVec4(0,0,0,0), vdbVec4 v_max=vdbVec4(1,1,1,1));

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Shaders
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbLoadShader(int slot, const char *fragment_shader_source_string);
void    vdbBeginShader(int slot);
void    vdbEndShader();
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
void    vdbUniformRowMajMatrix4fv(const char *name, float *x);
void    vdbUniformRowMajMatrix3fv(const char *name, float *x);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Render targets
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbBeginRenderTarget(int slot, int width, int height, vdbTextureFormat format, int depth_bits=0, int stencil_bits=0);
void    vdbEndRenderTarget(int slot);
void    vdbUnbindRenderTarget();
void    vdbBindRenderTarget(int slot, vdbTextureFilter filter=VDB_LINEAR, vdbTextureWrap wrap=VDB_CLAMP);
void    vdbDrawRenderTarget(int slot, vdbTextureFilter filter=VDB_LINEAR, vdbTextureWrap wrap=VDB_CLAMP);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Widgets
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float   vdbSliderFloat(const char *name, float vmin, float vmax, float v_init);
int     vdbSliderInt(const char *name, int vmin, int vmax, int v_init);
bool    vdbCheckbox(const char *name, bool init);
bool    vdbRadioButton(const char *name);
bool    vdbButton(const char *name);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Render scaler
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbBeginRenderScale(int down, int up);
void    vdbBeginRenderScale(int width, int height, int up);
void    vdbEndRenderScale();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Low-level functionality
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbDetachGLContext();
void    vdbStepOnce();
void    vdbStepOver();
bool    vdbBeginBreak(const char *label);
void    vdbEndBreak();
bool    vdbIsFirstFrame();
bool    vdbIsDifferentLabel();
vdbVec2 vdbGetRenderScale(); // See FAQ:RenderScale below
vdbVec2 vdbGetRenderOffset(); // See FAQ:RenderOffset below

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// § Logging
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void    vdbClearLog(const char *label=0);
void    vdbLogScalar(const char *label, float x, bool overwrite=false); // create new scalar
void    vdbLogArray(const char *label, float x); // append one element to existing array (or create new)
void    vdbLogArray(const char *label, float *x, int columns, bool append=false); // create new array or append to existing (append=true)
void    vdbLogMatrix(const char *label, float *x, int rows, int columns); // create new matrix
void    vdbLogMatrix(const char *label, float **x, int rows, int columns); // create new matrix
void    vdbLogMatrixRow(const char *label, float *x, int columns); // append row to existing matrix (or create new)
void    vdbLogMatrixCol(const char *label, float *x, int rows); // append column to existing matrix (or create new)
void    vdbLogMatrixTranspose(const char *label, float *x, int rows, int columns); // create new matrix
void    vdbLogMatrixTranspose(const char *label, float **x, int rows, int columns); // create new matrix

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

/*
FREQUENTLY ASKED QUESTIONS
==========================

Q: What are the scaling factors 2/2, 2/4, 4/4, 8/8, etc. in the Settings menu tab?
A:  The render scale lets you render at a lower resolution. Useful for increasing
    framerate or saving battery life. vdbGetRenderScale() returns the active scale
    factor (1/1 = 1.0, 1/2 = 0.5, etc.).

    Scaling options a/b where a > 1 enables temporal multi-sampling, where multiple
    frames are interleaved together over time to form a high resolution result. For
    example: 2/2 means render frames at 1/2 resolution and upsample 2x. The result
    is a original-resolution output that takes 4 frames to fully flush.

    This is done by having each frame render a slightly different view of the scene
    by translating incoming geometry by a subpixel offset, which is equivalent to
    changing the pixel sampling center. To get this effect when using custom shaders
    (i.e. NOT when using the immediate API), you should use vdbGetRenderOffset().

    See test.cpp for example usage in a fragment shader-based ray-tracer.
*/
