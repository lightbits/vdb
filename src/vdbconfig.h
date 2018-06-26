// You may copy this file into your project and #include it before vdb.h
// if you want to overwrite the default configuration.

// vdbProjection and vdbPushMatrix both take a matrix in float* format,
// which can either be interpreted as column-major or row-major order.
//
// VDB_MATRIX_COLUMN_MAJOR:
// Memory layout follows row, i.e.
//   float m[] = {A,B,C,D}; -> |A B|
//                             |C D|
//
// VDB_MATRIX_ROW_MAJOR:
// Memory layout follows column, i.e.
//   float m[] = {A,B,C,D}; -> |A C|
//                             |B D|
//
#define VDB_MATRIX_COLUMN_MAJOR
// #define VDB_MATRIX_ROW_MAJOR

#define VDB_FONT_HEIGHT        18.0f

#define VDB_FLIP_IMAGE_TEXEL_Y 0

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

// The state of ImGui windows is remembered between sessions.
// This path specifies the path (relative to working directory)
// where the information is stored.
#define VDB_IMGUI_INI_FILENAME "./imgui.ini"
