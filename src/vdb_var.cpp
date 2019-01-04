enum quick_var_type_t { VAR_TYPE_FLOAT, VAR_TYPE_INT, VAR_TYPE_TOGGLE, VAR_TYPE_RADIO };

struct quick_var_t
{
    const char *name;
    quick_var_type_t type;
    struct float_var_t { float value; float vmin; float vmax; } f;
    struct int_var_t { int value; int vmin; int vmax; } i;
    struct bool_var_t { bool enabled; } b;
    struct radio_var_t { int index; } r;
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
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 8.0f);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(ImVec2(0.4f*vdbGetWindowWidth(), -1));
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("Quick Var##vdb", NULL, flags);
        // ImGui::Begin("Debug##Default");
        for (int i = 0; i < var_index; i++)
        {
            quick_var_t *var = vars + i;
            if (var->type == VAR_TYPE_FLOAT)
                ImGui::SliderFloat(var->name, &var->f.value, var->f.vmin, var->f.vmax);
            else if (var->type == VAR_TYPE_INT)
                ImGui::SliderInt(var->name, &var->i.value, var->i.vmin, var->i.vmax);
            else if (var->type == VAR_TYPE_TOGGLE)
                ImGui::Checkbox(var->name, &var->b.enabled);
            else if (var->type == VAR_TYPE_RADIO)
                ImGui::RadioButton(var->name, &active_radiobutton_index, var->r.index);
        }
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }
}

float vdbSlider1f(const char *name, float vmin, float vmax, float vinit)
{
    using namespace quick_var;
    quick_var_t *var = vars + (var_index++);
    if (vdbIsFirstFrame())
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
    if (vdbIsFirstFrame())
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
    if (vdbIsFirstFrame())
    {
        var->b.enabled = init;
        var->name = name;
        var->type = VAR_TYPE_TOGGLE;
    }
    return var->b.enabled;
}
bool vdbRadio(const char *name)
{
    using namespace quick_var;
    quick_var_t *var = vars + (var_index++);
    if (vdbIsFirstFrame())
    {
        var->r.index = radiobutton_index++;
        var->name = name;
        var->type = VAR_TYPE_RADIO;
    }
    return var->r.index == active_radiobutton_index;
}
