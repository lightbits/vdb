struct vdb_ruler_mode_t
{
    vdbVec2 a,b;
    vdbVec2 mouse;
};
static vdb_ruler_mode_t vdb_ruler;

static void vdbRulerMode(bool mouse_left_down, vdbVec2 mouse)
{
    static bool dragging = false;
    vdb_ruler.mouse = mouse;
    if (mouse_left_down)
    {
        if (!dragging)
        {
            dragging = true;
            vdb_ruler.a = mouse;
            vdb_ruler.b = mouse;
        }
        else
        {
            vdb_ruler.b = mouse;
        }
    }
    else
    {
        dragging = false;
    }
}

static void vdbRulerModePresent()
{
    vdbVec2 a = vdb_ruler.a;
    vdbVec2 b = vdb_ruler.b;
    vdbVec2 mouse = vdb_ruler.mouse;
    vdbVec2 ndc_a = vdbWindowToNDC(a.x, a.y);
    vdbVec3 model_a = vdbNDCToModel(ndc_a.x, ndc_a.y);
    vdbVec2 ndc_b = vdbWindowToNDC(b.x, b.y);
    vdbVec3 model_b = vdbNDCToModel(ndc_b.x, ndc_b.y);
    float dx = model_b.x - model_a.x;
    float dy = model_b.y - model_a.y;
    float distance = sqrtf(dx*dx + dy*dy);
    float distance_px = sqrtf((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|
        ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoInputs|
        ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoFocusOnAppearing|
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("##RulerDrawWindow", NULL, flags);
    ImDrawList *draw = ImGui::GetWindowDrawList();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImU32 fg = IM_COL32(255,255,255,255);
    ImU32 bg = IM_COL32(0,0,0,255);

    if (distance_px > 1.0f)
    {
        float thickness = 2.0f;
        draw->AddLine(ImVec2(a.x,a.y), ImVec2(b.x,b.y), bg, thickness+2.0f);
        draw->AddCircleFilled(ImVec2(a.x,a.y), 5.0f, bg);
        draw->AddCircleFilled(ImVec2(b.x,b.y), 5.0f, bg);
        draw->AddLine(ImVec2(a.x,a.y), ImVec2(b.x,b.y), fg, thickness);
        draw->AddCircleFilled(ImVec2(a.x,a.y), 4.0f, fg);
        draw->AddCircleFilled(ImVec2(b.x,b.y), 4.0f, fg);
    }

    {
        float x = 3.0f;
        char text[1024];
        snprintf(text, sizeof(text), "%d, %d px", (int)mouse.x, (int)mouse.y);
        draw->AddText(ImVec2(x+1,1), bg, text);
        draw->AddText(ImVec2(x,0), fg, text);
        x += 80.0f;

        snprintf(text, sizeof(text), "%d px", (int)distance_px);
        draw->AddText(ImVec2(x+1,1), bg, text);
        draw->AddText(ImVec2(x,0), fg, text);
        x += 80.0f;

        snprintf(text, sizeof(text), "%g user", distance);
        draw->AddText(ImVec2(x+1,1), bg, text);
        draw->AddText(ImVec2(x,0), fg, text);
        x += 80.0f;
    }
}
