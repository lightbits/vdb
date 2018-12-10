struct vdbMouseOverState
{
    int index;
    int closest_index;
    int prev_closest_index;
    float closest_distance;
    float closest_x;
    float closest_y;
    float closest_z;
};
static vdbMouseOverState vdb_map = {0};
void vdbResetMouseOverState()
{
    vdb_map.closest_distance = FLT_MAX;
    vdb_map.prev_closest_index = vdb_map.closest_index;
    vdb_map.index = 0;
}

bool vdbWasMouseOver(float x, float y, float z, float w)
{
    // todo: find closest in z
    vdbVec2 ndc = vdbModelToNDC(x, y, z, w);
    vdbVec2 win = vdbNDCToWindow(ndc.x, ndc.y);

    float dx = win.x - (float)vdb.mouse.x;
    float dy = win.y - (float)vdb.mouse.y;

    float distance = dx*dx + dy*dy;

    if (distance < vdb_map.closest_distance)
    {
        vdb_map.closest_index = vdb_map.index;
        vdb_map.closest_x = x;
        vdb_map.closest_y = y;
        vdb_map.closest_z = z;
        vdb_map.closest_distance = distance;
    }

    bool active_last_frame = vdb_map.index == vdb_map.prev_closest_index;
    vdb_map.index++;
    return active_last_frame;
}
int vdbGetMouseOverIndex(float *x, float *y, float *z)
{
    if (x) *x = vdb_map.closest_x;
    if (y) *y = vdb_map.closest_y;
    if (z) *z = vdb_map.closest_z;
    return vdb_map.closest_index;
}
vdbVec2 vdbGetMousePos()      { vdbVec2 result((float)vdb.mouse.x, (float)vdb.mouse.y); return result; }
vdbVec2 vdbGetMousePosNDC()   { return vdb.mouse.ndc; }
vdbVec3 vdbGetMousePosModel(float depth) { vdbVec2 ndc = vdbGetMousePosNDC(); vdbVec3 model = vdbNDCToModel(ndc.x, ndc.y, depth); return model; }
float vdbGetMouseWheel() { return ImGui::GetIO().WantCaptureMouse ? 0.0f : vdb.mouse.wheel; }

bool vdbWasMouseLeftPressed()    { return vdb.mouse.left.pressed    && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseRightPressed()   { return vdb.mouse.right.pressed   && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseMiddlePressed()  { return vdb.mouse.middle.pressed  && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseLeftReleased()   { return vdb.mouse.left.released   && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseRightReleased()  { return vdb.mouse.right.released  && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseMiddleReleased() { return vdb.mouse.middle.released && !ImGui::GetIO().WantCaptureMouse; }
bool vdbIsMouseLeftDown()       { return vdb.mouse.left.down       && !ImGui::GetIO().WantCaptureMouse; }
bool vdbIsMouseRightDown()      { return vdb.mouse.right.down      && !ImGui::GetIO().WantCaptureMouse; }
bool vdbIsMouseMiddleDown()     { return vdb.mouse.middle.down     && !ImGui::GetIO().WantCaptureMouse; }
