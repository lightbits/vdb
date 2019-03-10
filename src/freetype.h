// Obs! This file must be included before ImGui that uses freetype functions
// because we redefine them. This is so that we don't have to change the ImGui
// source code to use differently named functions (allow for easy updates).
#if VDB_IMGUI_FREETYPE_DYNAMIC==1
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SYNTHESIS_H
typedef FT_Error (*PFNFTINITFREETYPE)(FT_Library *alibrary);
typedef FT_Error (*PFNFTRENDERGLYPH)(FT_GlyphSlot slot, FT_Render_Mode render_mode);
typedef FT_Error (*PFNFTNEWMEMORYFACE)(FT_Library library, const FT_Byte* file_base, FT_Long file_size, FT_Long face_index, FT_Face *aface);
typedef FT_Error (*PFNFTSELECTCHARMAP)(FT_Face face, FT_Encoding encoding);
typedef FT_Error (*PFNFTDONEFACE)(FT_Face face);
typedef FT_Error (*PFNFTDONEFREETYPE)(FT_Library library);
typedef FT_Error (*PFNFTREQUESTSIZE)(FT_Face face, FT_Size_Request req);
typedef FT_UInt (*PFNFTGETCHARINDEX)(FT_Face face, FT_ULong charcode);
typedef FT_Error (*PFNFTLOADGLYPH)(FT_Face face, FT_UInt glyph_index, FT_Int32 load_flags);
typedef void (*PFNFTGLYPHSLOTEMBOLDEN)(FT_GlyphSlot slot);
typedef void (*PFNFTGLYPHSLOTOBLIQUE)(FT_GlyphSlot slot);
typedef FT_Error (*PFNFTGETGLYPH)(FT_GlyphSlot slot, FT_Glyph *aglyph);
typedef FT_Error (*PFNFTGLYPHTOBITMAP)(FT_Glyph* the_glyph, FT_Render_Mode render_mode, FT_Vector* origin, FT_Bool destroy);
typedef void (*PFNFTDONEGLYPH)(FT_Glyph glyph);
PFNFTINITFREETYPE VDB_FT_Init_FreeType;
PFNFTRENDERGLYPH VDB_FT_Render_Glyph;
PFNFTNEWMEMORYFACE VDB_FT_New_Memory_Face;
PFNFTSELECTCHARMAP VDB_FT_Select_Charmap;
PFNFTDONEFACE VDB_FT_Done_Face;
PFNFTDONEFREETYPE VDB_FT_Done_FreeType;
PFNFTREQUESTSIZE VDB_FT_Request_Size;
PFNFTGETCHARINDEX VDB_FT_Get_Char_Index;
PFNFTLOADGLYPH VDB_FT_Load_Glyph;
PFNFTGLYPHSLOTEMBOLDEN VDB_FT_GlyphSlot_Embolden;
PFNFTGLYPHSLOTOBLIQUE VDB_FT_GlyphSlot_Oblique;
PFNFTGETGLYPH VDB_FT_Get_Glyph;
PFNFTGLYPHTOBITMAP VDB_FT_Glyph_To_Bitmap;
PFNFTDONEGLYPH VDB_FT_Done_Glyph;
#define FT_Init_FreeType VDB_FT_Init_FreeType
#define FT_Render_Glyph VDB_FT_Render_Glyph
#define FT_New_Memory_Face VDB_FT_New_Memory_Face
#define FT_Select_Charmap VDB_FT_Select_Charmap
#define FT_Done_Face VDB_FT_Done_Face
#define FT_Done_FreeType VDB_FT_Done_FreeType
#define FT_Request_Size VDB_FT_Request_Size
#define FT_Get_Char_Index VDB_FT_Get_Char_Index
#define FT_Load_Glyph VDB_FT_Load_Glyph
#define FT_GlyphSlot_Embolden VDB_FT_GlyphSlot_Embolden
#define FT_GlyphSlot_Oblique VDB_FT_GlyphSlot_Oblique
#define FT_Get_Glyph VDB_FT_Get_Glyph
#define FT_Glyph_To_Bitmap VDB_FT_Glyph_To_Bitmap
#define FT_Done_Glyph VDB_FT_Done_Glyph
#endif

#include "imgui/imgui_freetype.cpp"

bool TryLoadFreetype()
{
    static bool ok = false;
    static bool have_tried = false;
    if (have_tried)
        return ok;
    have_tried = true;

    static void *freetype = SDL_LoadObject("freetype.dll");
    if (freetype)
    {
        ok = true;
        FT_Init_FreeType      = (PFNFTINITFREETYPE)      SDL_LoadFunction(freetype, "FT_Init_FreeType");      if (!VDB_FT_Init_FreeType) ok = false;
        FT_Render_Glyph       = (PFNFTRENDERGLYPH)       SDL_LoadFunction(freetype, "FT_Render_Glyph");       if (!VDB_FT_Render_Glyph) ok = false;
        FT_New_Memory_Face    = (PFNFTNEWMEMORYFACE)     SDL_LoadFunction(freetype, "FT_New_Memory_Face");    if (!VDB_FT_New_Memory_Face) ok = false;
        FT_Select_Charmap     = (PFNFTSELECTCHARMAP)     SDL_LoadFunction(freetype, "FT_Select_Charmap");     if (!VDB_FT_Select_Charmap) ok = false;
        FT_Done_Face          = (PFNFTDONEFACE)          SDL_LoadFunction(freetype, "FT_Done_Face");          if (!VDB_FT_Done_Face) ok = false;
        FT_Done_FreeType      = (PFNFTDONEFREETYPE)      SDL_LoadFunction(freetype, "FT_Done_FreeType");      if (!VDB_FT_Done_FreeType) ok = false;
        FT_Request_Size       = (PFNFTREQUESTSIZE)       SDL_LoadFunction(freetype, "FT_Request_Size");       if (!VDB_FT_Request_Size) ok = false;
        FT_Get_Char_Index     = (PFNFTGETCHARINDEX)      SDL_LoadFunction(freetype, "FT_Get_Char_Index");     if (!VDB_FT_Get_Char_Index) ok = false;
        FT_Load_Glyph         = (PFNFTLOADGLYPH)         SDL_LoadFunction(freetype, "FT_Load_Glyph");         if (!VDB_FT_Load_Glyph) ok = false;
        FT_GlyphSlot_Embolden = (PFNFTGLYPHSLOTEMBOLDEN) SDL_LoadFunction(freetype, "FT_GlyphSlot_Embolden"); if (!VDB_FT_GlyphSlot_Embolden) ok = false;
        FT_GlyphSlot_Oblique  = (PFNFTGLYPHSLOTOBLIQUE)  SDL_LoadFunction(freetype, "FT_GlyphSlot_Oblique");  if (!VDB_FT_GlyphSlot_Oblique) ok = false;
        FT_Get_Glyph          = (PFNFTGETGLYPH)          SDL_LoadFunction(freetype, "FT_Get_Glyph");          if (!VDB_FT_Get_Glyph) ok = false;
        FT_Glyph_To_Bitmap    = (PFNFTGLYPHTOBITMAP)     SDL_LoadFunction(freetype, "FT_Glyph_To_Bitmap");    if (!VDB_FT_Glyph_To_Bitmap) ok = false;
        FT_Done_Glyph         = (PFNFTDONEGLYPH)         SDL_LoadFunction(freetype, "FT_Done_Glyph");         if (!VDB_FT_Done_Glyph) ok = false;
    }
    return ok;
}
