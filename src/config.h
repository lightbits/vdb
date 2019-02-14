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

// When enabled, ImGui will use FreeType for font rendering (less blurry for small
// font sizes). Unless VDB_IMGUI_FREETYPE_DYNAMIC is enabled, you need to manually
// link Freetype with your final executable. Precompiled library binaries for Win32
// and Win64 are included in vdb/lib/freetype.
#define VDB_IMGUI_FREETYPE         1

// If enabled, VDB will try to locate freetype.dll on startup and link it for you
// dynamically. This means you don't have to change the build system for your final
// executable, and only need to make sure that freetype.dll is accessible (e.g. in
// the same directory), and matches the version of the header files provided in
// vdb/lib/freetype/include. If freetype.dll is not found, freetype is not used.
#define VDB_IMGUI_FREETYPE_DYNAMIC 1

// For small font sizes I recommend using the Freetype renderer.
#define VDB_DEFAULT_FONT_SIZE  18

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
#define VDB_SAVE_SETTINGS_PERIOD 60*5

// The state of ImGui windows is remembered between sessions.
// This path specifies the path (relative to working directory)
// where the information is stored.
#define VDB_IMGUI_INI_FILENAME "./imgui.ini"

#define VDB_HOTKEY_FRAMEGRAB   (keys::pressed[SDL_SCANCODE_S] && keys::down[SDL_SCANCODE_LALT])
#define VDB_HOTKEY_WINDOW_SIZE (keys::pressed[SDL_SCANCODE_W] && keys::down[SDL_SCANCODE_LALT])
#define VDB_HOTKEY_SKETCH_MODE (keys::pressed[SDL_SCANCODE_D] && keys::down[SDL_SCANCODE_LALT])
#define VDB_HOTKEY_RULER_MODE  (keys::pressed[SDL_SCANCODE_R] && keys::down[SDL_SCANCODE_LALT])
#define VDB_HOTKEY_TOGGLE_MENU (keys::pressed[SDL_SCANCODE_M] && keys::down[SDL_SCANCODE_LALT])
#define VDB_HOTKEY_LOGS_WINDOW (keys::pressed[SDL_SCANCODE_L] && keys::down[SDL_SCANCODE_LALT])
#define VDB_HOTKEY_AUTO_STEP   (keys::pressed[SDL_SCANCODE_F6])
