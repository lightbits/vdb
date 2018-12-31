// COMPILE-TIME OPTIONS FOR VDB
// You may edit this file in your local copy of vdb if you want to overwrite
// the default configuration.

// vdbProjection and vdbPushMatrix both take a matrix in float* format,
// which can either be interpreted as column-major or row-major order.
//
// VDB_MATRIX_COLUMN_MAJOR:
//   float m[] = {A,B,C,D}; -> |A B|
//                             |C D|
//
// VDB_MATRIX_ROW_MAJOR:
//   float m[] = {A,B,C,D}; -> |A C|
//                             |B D|
//
// #define VDB_MATRIX_COLUMN_MAJOR
#define VDB_MATRIX_ROW_MAJOR

#define VDB_USE_FIXED_FUNCTION_PIPELINE 0

// Height of default font
#define VDB_FONT_HEIGHT        18.0f

// When set to 1, images drawn with vdbDrawImage have reversed y texture coordinate
#define VDB_FLIP_IMAGE_TEXEL_Y 0

// OpenGL version
#define VDB_GL_MAJOR           3
#define VDB_GL_MINOR           1

// Set to > 0 to get smooth edges on points, lines and triangles.
#define VDB_MULTISAMPLES       4

// Set to > 0 to be able to take screenshots with transparent backgrounds.
#define VDB_ALPHABITS          8

// Set to > 0 if you want to use the OpenGL stencil operations.
#define VDB_STENCILBITS        8

// Set to > 0 if you want to use OpenGL depth testing.
#define VDB_DEPTHBITS          24

// The size of the vdb window is remembered between sessions.
// This path specifies the path (relative to working directory)
// where the information is stored.
#define VDB_SETTINGS_FILENAME  "./vdb.ini"

// Number of frames to pass between saving settings to disk
#define VDB_SAVE_SETTINGS_PERIOD 64*5

// The state of ImGui windows is remembered between sessions.
// This path specifies the path (relative to working directory)
// where the information is stored.
#define VDB_IMGUI_INI_FILENAME "./imgui.ini"


// This shader can be used to render points as colored meshes. This gives
// you MSAA for free (if enabled), but you end up with non-perfect circles.
// This option might be faster than the quad shader, especially for low
// vertex counts.
#define VDB_POINT_SHADER_VERTEX

// This shader can be used to render points as pixel-perfect circles,
// using textured quads. This option might be slower than the vertex version
// especially if you restrict yourself to low vertex count.
// #define VDB_POINT_SHADER_QUAD

#define VDB_HOTKEY_FRAMEGRAB   keys::pressed[SDL_SCANCODE_S] && keys::down[SDL_SCANCODE_LALT]
#define VDB_HOTKEY_WINDOW_SIZE keys::pressed[SDL_SCANCODE_W] && keys::down[SDL_SCANCODE_LALT]
#define VDB_HOTKEY_SKETCH_MODE keys::pressed[SDL_SCANCODE_D] && keys::down[SDL_SCANCODE_LALT]
#define VDB_HOTKEY_RULER_MODE  keys::pressed[SDL_SCANCODE_R] && keys::down[SDL_SCANCODE_LALT]
