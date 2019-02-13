namespace mouse_over
{
    static int index;
    static int closest_index;
    static int prev_closest_index;
    static float closest_distance;
    static float closest_x;
    static float closest_y;
    static float closest_z;
}

namespace mouse
{
    static int x,y; // The position of the mouse in the client area in screen coordinates where (0,0):top-left
    static vdbVec2 ndc; // -||- in normalized device coordinates where (-1,-1):bottom-left (+1,+1):top-right
    static float wheel;
    static struct button_t
    {
        bool pressed,released,down;
    } left,right,middle;

    static void NewFrame()
    {
        // Note: this must be called after viewport is set up
        ndc = vdbWindowToNDC((float)x, (float)y);

        mouse_over::closest_distance = FLT_MAX;
        mouse_over::prev_closest_index = mouse_over::closest_index;
        mouse_over::index = 0;
        mouse_over::closest_index = -1;
    }
}

bool vdbWasMouseOver(float x, float y, float z, float w)
{
    // todo: find closest in z
    vdbVec2 ndc = vdbModelToNDC(x, y, z, w);
    vdbVec2 win = vdbNDCToWindow(ndc.x, ndc.y);

    float dx = win.x - (float)mouse::x;
    float dy = win.y - (float)mouse::y;

    float distance = dx*dx + dy*dy;

    if (distance < mouse_over::closest_distance)
    {
        mouse_over::closest_index = mouse_over::index;
        mouse_over::closest_x = x;
        mouse_over::closest_y = y;
        mouse_over::closest_z = z;
        mouse_over::closest_distance = distance;
    }

    bool active_last_frame = mouse_over::index == mouse_over::prev_closest_index;
    mouse_over::index++;
    return active_last_frame;
}
int vdbGetMouseOverIndex(float *x, float *y, float *z)
{
    if (x) *x = mouse_over::closest_x;
    if (y) *y = mouse_over::closest_y;
    if (z) *z = mouse_over::closest_z;
    return mouse_over::closest_index;
}

vdbVec2 vdbGetMousePos()
{
    return vdbVec2((float)mouse::x, (float)mouse::y);
}

vdbVec2 vdbGetMousePosNDC()
{
    return mouse::ndc;
}

vdbVec3 vdbGetMousePosModel(float depth)
{
    vdbVec2 ndc = vdbGetMousePosNDC();
    vdbVec3 model = vdbNDCToModel(ndc.x, ndc.y, depth);
    return model;
}

float vdbGetMouseWheel()
{
    return ImGui::GetIO().WantCaptureMouse ? 0.0f : mouse::wheel;
}

bool vdbWasMouseLeftPressed()    { return mouse::left.pressed    && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseRightPressed()   { return mouse::right.pressed   && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseMiddlePressed()  { return mouse::middle.pressed  && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseLeftReleased()   { return mouse::left.released   && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseRightReleased()  { return mouse::right.released  && !ImGui::GetIO().WantCaptureMouse; }
bool vdbWasMouseMiddleReleased() { return mouse::middle.released && !ImGui::GetIO().WantCaptureMouse; }
bool vdbIsMouseLeftDown()        { return mouse::left.down       && !ImGui::GetIO().WantCaptureMouse; }
bool vdbIsMouseRightDown()       { return mouse::right.down      && !ImGui::GetIO().WantCaptureMouse; }
bool vdbIsMouseMiddleDown()      { return mouse::middle.down     && !ImGui::GetIO().WantCaptureMouse; }
