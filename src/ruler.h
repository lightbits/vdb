namespace ruler
{
    static bool active;
    static bool dragging;
    static vdbVec2 mouse; // window coordinates (ImGui coordinates)
    static vdbVec2 a,b; // window coordinates (ImGui coordinates)

    // Results
    static float distance_user;
    static float distance_pixels;

    static void BeginFrame();
    static void EndFrame();
    static void DrawOverlay();
}

static void ruler::BeginFrame()
{
    if (VDB_HOTKEY_RULER_MODE)
        active = !active;

    if (!active)
        return;

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

    vdbVec2 ndc_a = vdbWindowToNDC(a.x, a.y);
    vdbVec3 model_a = vdbNDCToModel(ndc_a.x, ndc_a.y);
    vdbVec2 ndc_b = vdbWindowToNDC(b.x, b.y);
    vdbVec3 model_b = vdbNDCToModel(ndc_b.x, ndc_b.y);
    float dx = model_b.x - model_a.x;
    float dy = model_b.y - model_a.y;
    distance_user = sqrtf(dx*dx + dy*dy);
    distance_pixels = sqrtf((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));
}

static void ruler::DrawOverlay()
{
    if (!active)
        return;

    ImDrawList *draw = ImGui::GetOverlayDrawList();

    ImU32 fg = IM_COL32(255,255,255,255);
    ImU32 bg = IM_COL32(0,0,0,255);

    if (distance_pixels > 1.0f)
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
    ImGui::Text("%4d, %4d px", (int)mouse.x, (int)mouse.y);
    ImGui::Separator();
    ImGui::Text("%4d px", (int)distance_pixels);
    ImGui::Separator();
    ImGui::Text("%g user", distance_user);
    ImGui::EndMainMenuBar();
}
