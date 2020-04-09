enum widget_type_t
{
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_FLOAT,
    WIDGET_TYPE_INT,
    WIDGET_TYPE_CHECKBOX,
};

struct widget_t
{
    const char *name;
    widget_type_t type;
    bool changed;
    bool deactivated;
    int position;
    struct float_var_t  { float value; float vmin; float vmax; const char *format; };
    struct int_var_t    { int value; int vmin; int vmax; };
    struct toggle_var_t { bool enabled; };
    union
    {
         float_var_t f;
         int_var_t i;
         toggle_var_t t;
    };
};

namespace widgets_panel
{
    enum { MAX_WIDGETS = 1024 };
    static widget_t widgets[MAX_WIDGETS];
    static int num_widgets = 0; // Number of variables for the current frame (a uniquely labelled begin/end block)
    static int selected = -1;

    static widget_t *GetWidget(const char *name, widget_type_t type)
    {
        for (int i = 0; i < num_widgets; i++)
            if (strcmp(widgets[i].name, name) == 0 && widgets[i].type == type)
                return &widgets[i];
        return NULL;
    }

    static void NewFrame()
    {
        num_widgets = 0;
        selected = -1;
    }

    static void BeginFrame()
    {
        if (!ImGui::IsMouseDown(0))
            selected = -1;
    }

    static void EndFrame()
    {
        if (num_widgets == 0)
            return;
        static bool is_hovered = false;

        struct index_t { int index; int position; };
        static index_t indices[MAX_WIDGETS];
        for (int i = 0; i < num_widgets; i++)
        {
            indices[i].index = i;
            indices[i].position = widgets[i].position;
        }
        qsort(indices, num_widgets, sizeof(index_t), [](const void *pa, const void *pb){
            const index_t *a = (const index_t*)pa;
            const index_t *b = (const index_t*)pb;
            return a->position < b->position ? -1 : +1;
        });

        // Set style
        vdb_style_t style = GetStyle();
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, is_hovered ? 1.0f : 0.3f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(style.text.x, style.text.y, style.text.z, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(ImVec2(0.0f, 5.0f + ui::main_menu_bar_height));
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
        for (int j = 0; j < num_widgets; j++)
        {
            int i = indices[j].index;
            widget_t &w = widgets[i];

            // Drag handle
            ImGui::BeginGroup();
            ImGui::Text("::");
            ImGui::SameLine();
            if      (w.type == WIDGET_TYPE_FLOAT)    w.changed = ImGui::SliderFloat(w.name, &w.f.value, w.f.vmin, w.f.vmax, w.f.format);
            else if (w.type == WIDGET_TYPE_INT)      w.changed = ImGui::SliderInt(w.name, &w.i.value, w.i.vmin, w.i.vmax);
            else if (w.type == WIDGET_TYPE_CHECKBOX) w.changed = ImGui::Checkbox(w.name, &w.t.enabled);
            else if (w.type == WIDGET_TYPE_BUTTON)   w.changed = ImGui::Button(w.name);
            w.deactivated = ImGui::IsItemDeactivatedAfterEdit();
            ImGui::EndGroup();

            if (ImGui::IsItemHovered() && ImGui::IsMouseDown(0) && selected == -1)
                selected = i;

            if (ImGui::IsItemHovered() && ImGui::IsMouseDown(0) && selected >= 0 && i != selected)
            {
                int temp = widgets[selected].position;
                widgets[selected].position = widgets[i].position;
                widgets[i].position = temp;
            }
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
    using namespace widgets_panel;
    widget_t *widget = GetWidget(name, WIDGET_TYPE_FLOAT);
    if (!widget)
    {
        assert(num_widgets < MAX_WIDGETS && "Reached maximum number of widgets");
        widget = &widgets[num_widgets++];
        widget->position = num_widgets;
        widget->changed = false;
        widget->f.value = vinit;
        widget->f.vmin = vmin;
        widget->f.vmax = vmax;
        widget->f.format = format;
        widget->name = name;
        widget->type = WIDGET_TYPE_FLOAT;
    }
    return widget->f.value;
}
int vdbSliderInt(const char *name, int vmin, int vmax, int vinit)
{
    using namespace widgets_panel;
    widget_t *widget = GetWidget(name, WIDGET_TYPE_INT);
    if (!widget)
    {
        assert(num_widgets < MAX_WIDGETS && "Reached maximum number of widgets");
        widget = &widgets[num_widgets++];
        widget->position = num_widgets;
        widget->changed = false;
        widget->i.value = vinit;
        widget->i.vmin = vmin;
        widget->i.vmax = vmax;
        widget->name = name;
        widget->type = WIDGET_TYPE_INT;
    }
    return widget->i.value;
}
bool vdbCheckbox(const char *name, bool init)
{
    using namespace widgets_panel;
    widget_t *widget = GetWidget(name, WIDGET_TYPE_CHECKBOX);
    if (!widget)
    {
        assert(num_widgets < MAX_WIDGETS && "Reached maximum number of widgets");
        widget = &widgets[num_widgets++];
        widget->position = num_widgets;
        widget->changed = false;
        widget->t.enabled = init;
        widget->name = name;
        widget->type = WIDGET_TYPE_CHECKBOX;
    }
    return widget->t.enabled;
}
bool vdbButton(const char *name)
{
    using namespace widgets_panel;
    widget_t *widget = GetWidget(name, WIDGET_TYPE_BUTTON);
    if (!widget)
    {
        assert(num_widgets < MAX_WIDGETS && "Reached maximum number of widgets");
        widget = &widgets[num_widgets++];
        widget->position = num_widgets;
        widget->changed = false;
        widget->name = name;
        widget->type = WIDGET_TYPE_BUTTON;
    }
    return widget->changed;
}
bool vdbWereItemsEdited()
{
    // using namespace widgets_panel;
    // bool result = false;
    // for (int i = edit_index; i < var_index; i++)
    //     result |= vars[i].changed;
    // edit_index = var_index;
    // return result;
    return false;
}
bool vdbWereItemsDeactivated()
{
    // using namespace widgets_panel;
    // bool result = false;
    // for (int i = edit_index; i < var_index; i++)
    //     result |= vars[i].deactivated;
    // edit_index = var_index;
    // return result;
    return false;
}
