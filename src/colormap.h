#include "data/colormap_data.h"

namespace colormap
{
    static int current_colormap = 0;
    static int current_color = 0;

    static vdbColormapData *GetColormapData()
    {
        assert(current_colormap >= 0);
        assert(current_colormap < NUM_COLORMAPS);
        return &vdb_colormap_data[current_colormap];
    }

    static void NewFrame()
    {
        current_colormap = 0;
    }

    static void BeginFrame()
    {
        current_color = 0;
    }
}

vdbVec3 vdbGetForegroundColor()
{
    if (settings.global_theme == VDB_DARK_THEME)
        return vdbVec3(1,1,1);
    else
        return vdbVec3(0,0,0);
}

vdbVec3 vdbGetBackgroundColor()
{
    if (settings.global_theme == VDB_DARK_THEME)
        return vdbVec3(VDB_DARK_THEME_BACKGROUND);
    else
        return vdbVec3(VDB_BRIGHT_THEME_BACKGROUND);
}

int vdbSetColormap(const char *name)
{
    int found = 0;
    for (int i = 0; i < NUM_COLORMAPS; i++)
    {
        if (strcmp(vdb_colormap_data[i].name, name) == 0)
        {
            found = i;
            break;
        }
    }
    assert(found >= 0);
    assert(found < NUM_COLORMAPS);
    colormap::current_colormap = found;
    colormap::current_color = 0;
    return vdb_colormap_data[found].num_colors;
}

void vdbColor4ub(unsigned char,unsigned char,unsigned char,unsigned char);

vdbVec4 vdbNextColor()
{
    int n = vdb_colormap_data[colormap::current_colormap].num_colors;
    int i = colormap::current_color;
    unsigned char *rgb = vdb_colormap_data[colormap::current_colormap].colors + 3*i;
    colormap::current_color = (colormap::current_color + 1) % n;
    unsigned char a8 = 255;
    vdbColor4ub(rgb[0], rgb[1], rgb[2], a8);
    return vdbVec4(rgb[0]/255.0f, rgb[1]/255.0f, rgb[2]/255.0f, a8/255.0f);
}

vdbVec4 vdbResetColor(int offset)
{
    colormap::current_color = 0;
    int n = vdb_colormap_data[colormap::current_colormap].num_colors;
    int i = colormap::current_color;
    unsigned char *rgb = vdb_colormap_data[colormap::current_colormap].colors + 3*i;
    unsigned char a8 = 255;
    vdbColor4ub(rgb[0], rgb[1], rgb[2], a8);
    return vdbVec4(rgb[0]/255.0f, rgb[1]/255.0f, rgb[2]/255.0f, a8/255.0f);
}

vdbVec4 vdbGetColor(float t, float alpha)
{
    int n = vdb_colormap_data[colormap::current_colormap].num_colors;
    int i = (int)(t*n);
    if (i < 0) i = 0;
    if (i > n-1) i = n-1;
    unsigned char *rgb = vdb_colormap_data[colormap::current_colormap].colors + 3*i;
    return vdbVec4(rgb[0]/255.0f, rgb[1]/255.0f, rgb[2]/255.0f, alpha);
}

vdbVec4 vdbGetColor(int i, float alpha)
{
    int n = vdb_colormap_data[colormap::current_colormap].num_colors;
    i = (colormap::current_color + i) % n;
    unsigned char *rgb = vdb_colormap_data[colormap::current_colormap].colors + 3*i;
    return vdbVec4(rgb[0]/255.0f, rgb[1]/255.0f, rgb[2]/255.0f, alpha);
}

void vdbColor(float t, float alpha)
{
    int n = vdb_colormap_data[colormap::current_colormap].num_colors;
    int i = (int)(t*n);
    if (i < 0) i = 0;
    if (i > n-1) i = n-1;
    unsigned char *rgb = vdb_colormap_data[colormap::current_colormap].colors + 3*i;
    int a8 = (int)(alpha*255.0f);
    if (a8 < 0) a8 = 0;
    if (a8 > 255) a8 = 255;
    vdbColor4ub(rgb[0], rgb[1], rgb[2], (unsigned char)a8);
}

void vdbColor(int i, float alpha)
{
    int n = vdb_colormap_data[colormap::current_colormap].num_colors;
    i = (colormap::current_color + i) % n;
    unsigned char *rgb = vdb_colormap_data[colormap::current_colormap].colors + 3*i;
    int a8 = (int)(alpha*255.0f);
    if (a8 < 0) a8 = 0;
    if (a8 > 255) a8 = 255;
    vdbColor4ub(rgb[0], rgb[1], rgb[2], (unsigned char)a8);
}

void vdbColorForeground(float alpha)
{
    vdbColor(vdbGetForegroundColor(), alpha);
}

void vdbColorBackground(float alpha)
{
    vdbColor(vdbGetBackgroundColor(), alpha);
}
