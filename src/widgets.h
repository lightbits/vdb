enum widget_type_t { VAR_TYPE_BUTTON, VAR_TYPE_FLOAT, VAR_TYPE_INT, VAR_TYPE_TOGGLE, VAR_TYPE_RADIO };

struct widget_t
{
    const char *name;
    widget_type_t type;
    bool changed;
    bool deactivated;
    struct float_var_t { float value; float vmin; float vmax; const char *format; };
    struct int_var_t { int value; int vmin; int vmax; };
    struct toggle_var_t { bool enabled; };
    struct radio_var_t { int index; };
    // struct button_var_t { };
    union
    {
         float_var_t f;
         int_var_t i;
         toggle_var_t t;
         radio_var_t r;
         // button_var_t b;
    };
};

namespace widgets
{
    enum { MAX_VARS = 1024 };
    static int var_index = 0;
    static int edit_index = 0;
    static int radiobutton_index = 0;
    static int active_radiobutton_index = 0;
    static widget_t vars[MAX_VARS];

    static void BeginFrame()
    {
        var_index = 0;
        edit_index = 0;
        radiobutton_index = 0;
    }

    static void EndFrame()
    {
        if (var_index == 0)
            return;
        vdb_style_t style = GetStyle();
        static bool is_hovered = false;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, is_hovered ? 1.0f : 0.3f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(style.text.x, style.text.y, style.text.z, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(ImVec2(-8.0f, 5.0f + ui::main_menu_bar_height));
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
            widget_t *var = vars + i;
            if (var->type == VAR_TYPE_FLOAT)
                var->changed = ImGui::SliderFloat(var->name, &var->f.value, var->f.vmin, var->f.vmax, var->f.format);
            else if (var->type == VAR_TYPE_INT)
                var->changed = ImGui::SliderInt(var->name, &var->i.value, var->i.vmin, var->i.vmax);
            else if (var->type == VAR_TYPE_TOGGLE)
                var->changed = ImGui::Checkbox(var->name, &var->t.enabled);
            else if (var->type == VAR_TYPE_RADIO)
                var->changed = ImGui::RadioButton(var->name, &active_radiobutton_index, var->r.index);
            else if (var->type == VAR_TYPE_BUTTON)
                var->changed = ImGui::Button(var->name);
            var->deactivated = ImGui::IsItemDeactivatedAfterEdit();
        }
        ImGui::PopItemWidth();
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }
}

float vdbSliderFloat(const char *name, float vmin, float vmax, float vinit, const char *format)
{
    using namespace widgets;
    widget_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() && vdbIsDifferentLabel()) // todo: better way to preserve variables for same-label windows
    {
        var->changed = false;
        var->f.value = vinit;
        var->f.vmin = vmin;
        var->f.vmax = vmax;
        var->f.format = format;
        var->name = name;
        var->type = VAR_TYPE_FLOAT;
    }
    return var->f.value;
}
int vdbSliderInt(const char *name, int vmin, int vmax, int vinit)
{
    using namespace widgets;
    widget_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() & vdbIsDifferentLabel())
    {
        var->changed = false;
        var->i.value = vinit;
        var->i.vmin = vmin;
        var->i.vmax = vmax;
        var->name = name;
        var->type = VAR_TYPE_INT;
    }
    return var->i.value;
}
bool vdbCheckbox(const char *name, bool init)
{
    using namespace widgets;
    widget_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() & vdbIsDifferentLabel())
    {
        var->changed = false;
        var->t.enabled = init;
        var->name = name;
        var->type = VAR_TYPE_TOGGLE;
    }
    return var->t.enabled;
}
bool vdbRadioButton(const char *name)
{
    using namespace widgets;
    widget_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() & vdbIsDifferentLabel())
    {
        var->changed = false;
        var->r.index = radiobutton_index++;
        var->name = name;
        var->type = VAR_TYPE_RADIO;
    }
    return var->r.index == active_radiobutton_index;
}
bool vdbButton(const char *name)
{
    using namespace widgets;
    widget_t *var = vars + (var_index++);
    if (vdbIsFirstFrame() & vdbIsDifferentLabel())
    {
        var->changed = false;
        var->name = name;
        var->type = VAR_TYPE_BUTTON;
    }
    return var->changed;
}
bool vdbWereItemsEdited()
{
    using namespace widgets;
    bool result = false;
    for (int i = edit_index; i < var_index; i++)
        result |= vars[i].changed;
    edit_index = var_index;
    return result;
}
bool vdbWereItemsDeactivated()
{
    using namespace widgets;
    bool result = false;
    for (int i = edit_index; i < var_index; i++)
        result |= vars[i].deactivated;
    edit_index = var_index;
    return result;
}
