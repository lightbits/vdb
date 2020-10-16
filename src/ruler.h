namespace ruler
{
    static bool active;
    static bool dragging;
    static vdbVec2 a,b,mouse; // window coordinates
    static vdbVec3 a_model,b_model,mouse_model; // model coordinates
    static float distance_model;
    static float distance;
    static int max_text_length;

    static void BeginFrame();
    static void EndFrame();
    static void DrawOverlay();
}

static void ruler::BeginFrame()
{
    if (VDB_HOTKEY_RULER_MODE)
        active = !active;

    if (ui::ruler_should_open)
    {
        active = true;
        ui::ruler_should_open = false;
    }

    if (!active)
    {
        max_text_length = 0;
        return;
    }

    if (keys::pressed[VDB_KEY_ESCAPE])
    {
        active = false;
        ui::escape_eaten = true;
        return;
    }

    mouse = vdbGetMousePos();
    if (vdbIsMouseLeftDown())
    {
        if (!dragging)
        {
            dragging = true;
            a = mouse;
            b = mouse;
        }
        else
        {
            b = mouse;
        }
    }
    else
    {
        dragging = false;
    }

    // force all subsequent calls to vdb{Is,Was}{Mouse,Key}{UpDownPressed} to return false
    ImGui::GetIO().WantCaptureKeyboard = true;
    ImGui::GetIO().WantCaptureMouse = true;
}

static void ruler::EndFrame()
{
    if (!active)
        return;

    vdbVec2 a_ndc = vdbWindowToNDC(a.x, a.y);
    vdbVec2 b_ndc = vdbWindowToNDC(b.x, b.y);
    vdbVec2 mouse_ndc = vdbWindowToNDC(mouse.x, mouse.y);
    a_model = vdbNDCToModel(a_ndc.x, a_ndc.y);
    b_model = vdbNDCToModel(b_ndc.x, b_ndc.y);
    mouse_model = vdbNDCToModel(mouse_ndc.x, mouse_ndc.y);
    float dx = b_model.x - a_model.x;
    float dy = b_model.y - a_model.y;
    float dz = b_model.z - a_model.z;
    distance_model = sqrtf(dx*dx + dy*dy + dz*dz);
    distance = sqrtf((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));
}

static void ruler::DrawOverlay()
{
    if (!active)
        return;

    ImDrawList *draw = ImGui::GetOverlayDrawList();

    ImU32 fg = IM_COL32(255,255,255,255);
    ImU32 bg = IM_COL32(0,0,0,255);

    if (distance > 1.0f)
    {
        float thickness = 2.0f;
        draw->AddLine(ImVec2(a.x,a.y), ImVec2(b.x,b.y), bg, thickness+2.0f);
        draw->AddCircleFilled(ImVec2(a.x,a.y), 5.0f, bg);
        draw->AddCircleFilled(ImVec2(b.x,b.y), 5.0f, bg);
        draw->AddLine(ImVec2(a.x,a.y), ImVec2(b.x,b.y), fg, thickness);
        draw->AddCircleFilled(ImVec2(a.x,a.y), 4.0f, fg);
        draw->AddCircleFilled(ImVec2(b.x,b.y), 4.0f, fg);
    }

    ImGui::BeginMainMenuBar();
    ImGui::Separator();
    ImGui::Text("Ruler: %10.6f (%d pixels)", distance_model, (int)distance);
    ImGui::EndMainMenuBar();
}
