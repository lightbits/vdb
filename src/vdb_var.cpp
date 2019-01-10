enum quick_var_type_t { VAR_TYPE_BUTTON, VAR_TYPE_FLOAT, VAR_TYPE_INT, VAR_TYPE_TOGGLE, VAR_TYPE_RADIO };

struct quick_var_t
{
    const char *name;
    quick_var_type_t type;
    union
    {
        struct float_var_t { float value; float vmin; float vmax; } f;
        struct int_var_t { int value; int vmin; int vmax; } i;
        struct toggle_var_t { bool enabled; } t;
        struct radio_var_t { int index; } r;
        struct button_var_t { bool was_pressed; } b;
    };
};

namespace quick_var
{
    enum { MAX_VARS = 1024 };
    static int var_index = 0;
    static int radiobutton_index = 0;
    static int active_radiobutton_index = 0;
    static quick_var_t vars[MAX_VARS];

    static void NewFrame()
    {
        var_index = 0;
        radiobutton_index = 0;
    }

    static void EndFrame()
    {
        if (var_index == 0)
            return;
        static bool is_hovered = false;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, is_hovered ? 1.0f : 0.3f);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(ImVec2(0.0f, 5.0f + uistuff::main_menu_bar_height));
        ImGui::SetNextWindowSize(ImVec2(200.0f, -1));
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("Quick Var##vdb", NULL, flags);
        is_hovered = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
        ImGui::PushItemWidth(120.0f);
        // ImGui::Begin("Debug##Default");
        for (int i = 0; i < var_index; i++)
        {
            quick_var_t *var = vars + i;
            if (var->type == VAR_TYPE_FLOAT)
                ImGui::SliderFloat(var->name, &var->f.value, var->f.vmin, var->f.vmax);
            else if (var->type == VAR_TYPE_INT)
                ImGui::SliderInt(var->name, &var->i.value, var->i.vmin, var->i.vmax);
            else if (var->type == VAR_TYPE_TOGGLE)
                ImGui::Checkbox(var->name, &var->t.enabled);
            else if (var->type == VAR_TYPE_RADIO)
                ImGui::RadioButton(var->name, &active_radiobutton_index, var->r.index);
            else if (var->type == VAR_TYPE_BUTTON)
                var->b.was_pressed = ImGui::Button(var->name);
        }
        ImGui::PopItemWidth();
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }
}

float vdbSlider1f(const char *name, float vmin, float vmax, float vinit)
{
    using namespace quick_var;
    quick_var_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() && vdbIsDifferentLabel()) // todo: better way to preserve variables for same-label windows
    {
        var->f.value = vinit;
        var->f.vmin = vmin;
        var->f.vmax = vmax;
        var->name = name;
        var->type = VAR_TYPE_FLOAT;
    }
    return var->f.value;
}
int vdbSlider1i(const char *name, int vmin, int vmax, int vinit)
{
    using namespace quick_var;
    quick_var_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() & vdbIsDifferentLabel())
    {
        var->i.value = vinit;
        var->i.vmin = vmin;
        var->i.vmax = vmax;
        var->name = name;
        var->type = VAR_TYPE_INT;
    }
    return var->i.value;
}
bool vdbToggle(const char *name, bool init)
{
    using namespace quick_var;
    quick_var_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() & vdbIsDifferentLabel())
    {
        var->t.enabled = init;
        var->name = name;
        var->type = VAR_TYPE_TOGGLE;
    }
    return var->t.enabled;
}
bool vdbRadio(const char *name)
{
    using namespace quick_var;
    quick_var_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() & vdbIsDifferentLabel())
    {
        var->r.index = radiobutton_index++;
        var->name = name;
        var->type = VAR_TYPE_RADIO;
    }
    return var->r.index == active_radiobutton_index;
}
bool vdbButton(const char *name)
{
    using namespace quick_var;
    quick_var_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() & vdbIsDifferentLabel())
    {
        var->name = name;
        var->type = VAR_TYPE_BUTTON;
    }
    return var->b.was_pressed;
}

void vdbPrintMatrix(const char *name, float *m, int rows, int cols, const char *fmt, bool transpose)
{
    if (!m)
        return;
    ImGui::PushID("vdbPrintMatrix");
    ImGui::Begin(name);
    for (int row = 0; row < rows; row++)
    for (int col = 0; col < cols; col++)
    {
        if (transpose) ImGui::Text(fmt, m[col + 4*row]);
        else ImGui::Text(fmt, m[row + 4*col]);
        if (col < cols-1)
            ImGui::SameLine();
    }
    ImGui::End();
    ImGui::PopID();
}
