namespace view_hints
{
    static float view_scale;           static bool view_scale_pending;
    static bool show_grid;             static bool show_grid_pending;
    static vdbCameraType camera_type;  static bool camera_type_pending;
    static vdbOrientation orientation; static bool orientation_pending;
}

static void ApplyViewHints()
{
    using namespace view_hints;
    if (view_scale_pending)
    {
        GetFrameSettings()->grid.grid_scale = view_scale;
        GetFrameSettings()->grid.dirty = true;
        view_scale_pending = false;
    }
    if (show_grid_pending)
    {
        GetFrameSettings()->grid.grid_visible = show_grid;
        GetFrameSettings()->grid.dirty = true;
        show_grid_pending = false;
    }
    if (camera_type_pending)
    {
        GetFrameSettings()->camera.type = camera_type;
        *GetCameraDirty() = true;
        camera_type_pending = false;
    }
    if (orientation_pending)
    {
        // obs! this must be applied after we apply the new camera type!
        *GetCameraUp() = orientation;
        orientation_pending = false;
    }
}

void vdbViewHint(vdbViewHintKey key, float value)
{
    if (key == VDB_VIEW_SCALE)
    {
        view_hints::view_scale = value;
        view_hints::view_scale_pending = true;
    }
}

void vdbViewHint(vdbViewHintKey key, bool value)
{
    if (key == VDB_SHOW_GRID)
    {
        view_hints::show_grid = value;
        view_hints::show_grid_pending = true;
    }
}

void vdbViewHint(vdbViewHintKey key, int value)
{
    if (key == VDB_CAMERA_TYPE &&
        (value == VDB_PLANAR ||
         value == VDB_TRACKBALL ||
         value == VDB_TURNTABLE))
    {
        view_hints::camera_type = value;
        view_hints::camera_type_pending = true;
    }
    else if (key == VDB_ORIENTATION &&
            (value == VDB_Y_UP || value == VDB_Y_DOWN ||
             value == VDB_X_UP || value == VDB_X_DOWN ||
             value == VDB_Z_UP || value == VDB_Z_DOWN))
    {
        view_hints::orientation = value;
        view_hints::orientation_pending = true;
    }
}
