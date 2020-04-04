struct vdb_style_t
{
    vdbVec3 x_axis;
    vdbVec3 y_axis;
    vdbVec3 z_axis;
    vdbVec3 grid;
    vdbVec3 cube;
    vdbVec3 clear;
    vdbVec3 text;
    float neg_alpha;
    float pos_alpha;
    float major_alpha;
    float minor_alpha;
};

static vdb_style_t GetStyle()
{
    vdb_style_t style;
    if (settings.global_theme == VDB_DARK_THEME)
    {
        style.x_axis = vdbVec3(VDB_DARK_THEME_X_AXIS);
        style.y_axis = vdbVec3(VDB_DARK_THEME_Y_AXIS);
        style.z_axis = vdbVec3(VDB_DARK_THEME_Z_AXIS);
        style.grid = vdbVec3(VDB_DARK_THEME_GRID_LINES);
        style.clear = vdbVec3(VDB_DARK_THEME_BACKGROUND);
        style.cube = vdbVec3(1.0f, 1.0f, 1.0f);
        style.text = vdbVec3(1.0f, 1.0f, 1.0f);
        style.neg_alpha = 0.3f;
        style.pos_alpha = 0.7f;
        style.minor_alpha = 0.3f;
        style.major_alpha = 0.5f;
    }
    else
    {
        style.x_axis = vdbVec3(VDB_BRIGHT_THEME_X_AXIS);
        style.y_axis = vdbVec3(VDB_BRIGHT_THEME_Y_AXIS);
        style.z_axis = vdbVec3(VDB_BRIGHT_THEME_Z_AXIS);
        style.grid = vdbVec3(VDB_BRIGHT_THEME_GRID_LINES);
        style.cube = vdbVec3(0.0f, 0.0f, 0.0f);
        style.clear = vdbVec3(VDB_BRIGHT_THEME_BACKGROUND);
        style.text = vdbVec3(0.0f, 0.0f, 0.0f);
        style.neg_alpha = 0.4f;
        style.pos_alpha = 0.6f;
        style.minor_alpha = 0.4f;
        style.major_alpha = 0.5f;
    }
    return style;
}
