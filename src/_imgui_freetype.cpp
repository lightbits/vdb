#if VDB_IMGUI_FREETYPE_DYNAMIC==1
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SYNTHESIS_H
typedef FT_Error (*PFNFTINITFREETYPE)(FT_Library *alibrary);
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
#else
#define VDB_FT_Init_FreeType FT_Init_FreeType
#define VDB_FT_New_Memory_Face FT_New_Memory_Face
#define VDB_FT_Select_Charmap FT_Select_Charmap
#define VDB_FT_Done_Face FT_Done_Face
#define VDB_FT_Done_FreeType FT_Done_FreeType
#define VDB_FT_Request_Size FT_Request_Size
#define VDB_FT_Get_Char_Index FT_Get_Char_Index
#define VDB_FT_Load_Glyph FT_Load_Glyph
#define VDB_FT_GlyphSlot_Embolden FT_GlyphSlot_Embolden
#define VDB_FT_GlyphSlot_Oblique FT_GlyphSlot_Oblique
#define VDB_FT_Get_Glyph FT_Get_Glyph
#define VDB_FT_Glyph_To_Bitmap FT_Glyph_To_Bitmap
#define VDB_FT_Done_Glyph FT_Done_Glyph
#endif

// This should be called before ImGui::GetTexDataAsRGBA32 (e.g. inside ImGui_ImplSdlGL3_CreateFontsTexture)
namespace imgui_freetype
{
    static void BuildFontAtlas()
    {
        #if VDB_IMGUI_FREETYPE_DYNAMIC==1
        static bool loaded = false;
        static void *freetype = SDL_LoadObject("freetype.dll");
        if (freetype)
        {
            loaded = true;
            VDB_FT_Init_FreeType      = (PFNFTINITFREETYPE)      SDL_LoadFunction(freetype, "FT_Init_FreeType");      if (!VDB_FT_Init_FreeType) loaded = false;
            VDB_FT_New_Memory_Face    = (PFNFTNEWMEMORYFACE)     SDL_LoadFunction(freetype, "FT_New_Memory_Face");    if (!VDB_FT_New_Memory_Face) loaded = false;
            VDB_FT_Select_Charmap     = (PFNFTSELECTCHARMAP)     SDL_LoadFunction(freetype, "FT_Select_Charmap");     if (!VDB_FT_Select_Charmap) loaded = false;
            VDB_FT_Done_Face          = (PFNFTDONEFACE)          SDL_LoadFunction(freetype, "FT_Done_Face");          if (!VDB_FT_Done_Face) loaded = false;
            VDB_FT_Done_FreeType      = (PFNFTDONEFREETYPE)      SDL_LoadFunction(freetype, "FT_Done_FreeType");      if (!VDB_FT_Done_FreeType) loaded = false;
            VDB_FT_Request_Size       = (PFNFTREQUESTSIZE)       SDL_LoadFunction(freetype, "FT_Request_Size");       if (!VDB_FT_Request_Size) loaded = false;
            VDB_FT_Get_Char_Index     = (PFNFTGETCHARINDEX)      SDL_LoadFunction(freetype, "FT_Get_Char_Index");     if (!VDB_FT_Get_Char_Index) loaded = false;
            VDB_FT_Load_Glyph         = (PFNFTLOADGLYPH)         SDL_LoadFunction(freetype, "FT_Load_Glyph");         if (!VDB_FT_Load_Glyph) loaded = false;
            VDB_FT_GlyphSlot_Embolden = (PFNFTGLYPHSLOTEMBOLDEN) SDL_LoadFunction(freetype, "FT_GlyphSlot_Embolden"); if (!VDB_FT_GlyphSlot_Embolden) loaded = false;
            VDB_FT_GlyphSlot_Oblique  = (PFNFTGLYPHSLOTOBLIQUE)  SDL_LoadFunction(freetype, "FT_GlyphSlot_Oblique");  if (!VDB_FT_GlyphSlot_Oblique) loaded = false;
            VDB_FT_Get_Glyph          = (PFNFTGETGLYPH)          SDL_LoadFunction(freetype, "FT_Get_Glyph");          if (!VDB_FT_Get_Glyph) loaded = false;
            VDB_FT_Glyph_To_Bitmap    = (PFNFTGLYPHTOBITMAP)     SDL_LoadFunction(freetype, "FT_Glyph_To_Bitmap");    if (!VDB_FT_Glyph_To_Bitmap) loaded = false;
            VDB_FT_Done_Glyph         = (PFNFTDONEGLYPH)         SDL_LoadFunction(freetype, "FT_Done_Glyph");         if (!VDB_FT_Done_Glyph) loaded = false;
        }
        if (!loaded)
            return;
        #endif
        ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts, 0);
    }
}
