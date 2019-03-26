/*
TODO
====
* Pre-compute ranges of log variables with same name
* Copy array to clipboard
*/

#pragma once
typedef int log_type_t;
typedef int plot_type_t;
enum log_type_ { VDB_LOG_UNKNOWN=0, VDB_LOG_SCALAR, VDB_LOG_ARRAY, VDB_LOG_MATRIX };
enum plot_type_ { VDB_PLOT_LINES=0, VDB_PLOT_HISTOGRAM };
struct log_t
{
    log_type_t type;
    char *label;
    union
    {
        float *data;
        float scalar;
    };
    int count;
    int capacity;

    log_t *prev;
    log_t *next;

    // gui
    bool selected;
    plot_type_t plot_type;
};

namespace logs
{
    static log_t *first;
    static log_t *last;

    static log_t *FindLog(const char *label, log_type_t type)
    {
        log_t *l = last;
        while (l)
        {
            if (l->type == type && strcmp(label, l->label) == 0)
                return l;
            l = l->prev;
        }
        return NULL;
    }

    static log_t *NewLog(const char *label, log_type_t type, log_t *insert_after=NULL)
    {
        log_t *l = new log_t;
        assert(l);
        l->type = type;
        l->label = strdup(label);
        l->count = 0;
        l->capacity = 0;
        l->data = NULL;
        l->selected = false;
        l->prev = NULL;
        l->next = NULL;
        l->selected = false;
        l->plot_type = VDB_PLOT_LINES;

        if (insert_after)
        {
            assert(first && last);
            if (last == insert_after)
            {
                insert_after->next = l;
                l->prev = insert_after;
                last = l;
            }
            else
            {
                l->next = insert_after->next;
                l->prev = insert_after;
                insert_after->next->prev = l;
                insert_after->next = l;
            }
        }
        else
        {
            if (last)
            {
                assert(first);
                l->prev = last;
                last->next = l;
                last = l;
            }
            else
            {
                first = l;
                last = l;
            }
        }

        return l;
    }

    static void DrawImGui();
}

void vdbClearLog(const char *label)
{
    using namespace logs;
    log_t *l = first;
    while (l)
    {
        assert(l->label);
        if (!label || strcmp(l->label, label) == 0)
        {
            free(l->label);
            if (l->capacity > 0)
            {
                assert(l->data);
                free(l->data);
            }
            if (l == first && l == last)
            {
                first = NULL;
                last = NULL;
                free(l);
                l = NULL;
            }
            else if (l == first)
            {
                l->next->prev = NULL;
                first = l->next;
                free(l);
                l = first;
            }
            else if (l == last)
            {
                l->prev->next = NULL;
                last = l->prev;
                free(l);
                l = NULL;
            }
            else
            {
                l->prev->next = l->next;
                l->next->prev = l->prev;
                log_t *next = l->next;
                free(l);
                l = next;
            }
        }
        else
        {
            l = l->next;
        }
    }
    assert(first->prev == NULL);
    assert(last->next == NULL);
}

void vdbLogScalar(const char *label, float x, bool overwrite)
{
    using namespace logs;
    log_t *l = FindLog(label, VDB_LOG_SCALAR);
    if (overwrite && l)
    {
        assert(l->capacity == 0);
        assert(l->count == 1);
        l->scalar = x;
    }
    else if ((l = NewLog(label, VDB_LOG_SCALAR, l)))
    {
        l->scalar = x;
        l->count = 1;
        l->capacity = 0;
    }
}

void vdbLogArray(const char *label, float x)
{
    using namespace logs;
    log_t *l = NULL;
    if ((l = FindLog(label, VDB_LOG_ARRAY)))
    {
        //
        // append one element
        //
        assert(l->data);
        assert(l->capacity > 0);
        assert(l->count >= 0);
        if (l->count + 1 > l->capacity)
        {
            int new_capacity = (l->capacity*3)/2;
            float *new_data = new float[new_capacity];
            // todo: memcpy, realloc
            for (int i = 0; i < l->count; i++)
                new_data[i] = l->data[i];
            free(l->data);
            l->data = new_data;
            l->capacity = new_capacity;
        }

        assert(l->count + 1 <= l->capacity);
        l->data[l->count++] = x;
    }
    else if ((l = NewLog(label, VDB_LOG_ARRAY)))
    {
        //
        // create new log
        //
        l->capacity = 16;
        l->data = new float[16];
        assert(l->data);
        l->data[l->count++] = x;
    }
}

void vdbLogArray(const char *label, float *x, int columns, bool append)
{
    using namespace logs;
    log_t *l = FindLog(label, VDB_LOG_ARRAY);
    if (append && l)
    {
        //
        // append 'columns' elements to existing log
        //
        assert(l->data);
        assert(l->capacity > 0);
        assert(l->count >= 0);
        if (l->count + columns > l->capacity)
        {
            int min_capacity = l->capacity + (int)columns;
            int new_capacity = (min_capacity*3)/2;
            float *new_data = new float[new_capacity];
            // todo: memcpy, realloc
            for (int i = 0; i < l->count; i++)
                new_data[i] = l->data[i];
            free(l->data);
            l->data = new_data;
            l->capacity = new_capacity;
        }

        // todo: memcpy
        assert(l->count + columns <= l->capacity);
        for (int i = 0; i < columns; i++)
            l->data[l->count++] = x[i];
    }
    else if ((l = NewLog(label, VDB_LOG_ARRAY, l)))
    {
        //
        // create new log (insert after any found log)
        //
        l->capacity = (int)columns;
        l->data = new float[columns];
        assert(l->data);
        // todo: memcpy
        for (int i = 0; i < columns; i++)
            l->data[l->count++] = x[i];
    }
}

static void logs::DrawImGui()
{
    #if 0
    float h_menu = uistuff::main_menu_bar_height;
    float w_window = ImGui::GetIO().DisplaySize.x;
    float h_window = ImGui::GetIO().DisplaySize.y;
    ImGui::SetNextWindowPos(ImVec2(0.0f, h_menu));
    ImGui::SetNextWindowSize(ImVec2(w_window, h_window));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f,0.1f,0.1f,1.0f));
    ImGui::Begin("##hiasd", NULL, flags);
    #else
    ImGui::Begin("Logs##vdb_logs");
    #endif

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
    ImGui::Columns(2);

    log_t *first = logs::first;
    while (first)
    {
        // todo: don't do this search for similar variables each frame
        // only need to do it when adding/deleting logs
        int num_same = 1;
        log_t *last_plus_one = first->next;
        while (last_plus_one && strcmp(last_plus_one->label, first->label) == 0)
        {
            last_plus_one = last_plus_one->next;
            num_same++;
        }

        ImGui::PushID(first);
        // ImGui::AlignTextToFramePadding();
        const char *fmt = NULL;
        if (first->type == VDB_LOG_SCALAR) fmt = "%s : scalar (%d)";
        else if (first->type == VDB_LOG_ARRAY) fmt = "%s : array (%d)";
        else fmt = "%s : unknown (%d)";
        bool node_open = ImGui::TreeNode("Object", fmt, first->label, num_same);
        // ImGui::AlignTextToFramePadding();
        if (node_open)
        {
            for (log_t *log = first; log != last_plus_one; log = log->next)
            {
                static char buffer[1024];
                char *stream = buffer;

                if (log->type == VDB_LOG_SCALAR)
                {
                    stream += sprintf(stream, "%g", log->scalar);
                }
                else if (log->type == VDB_LOG_ARRAY)
                {
                    // todo: safe sprintf
                    stream += sprintf(stream, "{ ");
                    for (int v = 0; v < log->count; v++)
                    {
                        const char *fmt = (v == 0) ? "%g"  : ", %g";
                        stream += sprintf(stream, fmt, log->data[v]);
                    }
                    stream += sprintf(stream, " }");
                }

                ImGui::PushID(log);
                ImGui::Selectable(buffer, &log->selected);
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();

        first = last_plus_one;
    }

    ImGui::NextColumn();

    static log_t *plot_to_configure = NULL;
    for (auto log = logs::first; log; log = log->next)
    {
        if (!log->selected)
            continue;
        auto log_type = log->type;
        auto plot_type = log->plot_type;

        if (log_type == VDB_LOG_SCALAR)
        {

        }
        else if (log_type == VDB_LOG_ARRAY)
        {
            if (plot_type == VDB_PLOT_LINES)
            {
                ImVec2 graph_size = ImVec2(0.0f, 64.0f);
                ImGui::PlotLines(log->label, log->data, log->count, 0, NULL, FLT_MAX, FLT_MAX, graph_size);
            }
            else if (plot_type == VDB_PLOT_HISTOGRAM)
            {
                ImVec2 graph_size = ImVec2(0.0f, 64.0f);
                ImGui::PlotHistogram(log->label, log->data, log->count, 0, NULL, FLT_MAX, FLT_MAX, graph_size);
            }
            else
            {
                assert(false && "Unhandled plot type");
            }
        }

        if (ImGui::IsItemClicked(1))
        {
            ImGui::OpenPopup("Plot");
            plot_to_configure = log;
        }
    }

    if (ImGui::BeginPopup("Plot"))
    {
        assert(plot_to_configure);
        ImGui::RadioButton("Lines", &plot_to_configure->plot_type, VDB_PLOT_LINES);
        ImGui::RadioButton("Histogram", &plot_to_configure->plot_type, VDB_PLOT_HISTOGRAM);
        ImGui::EndPopup();
    }
    else
    {
        plot_to_configure = NULL;
    }

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::PopStyleVar();
    #if 0
    ImGui::PopStyleColor();
    #endif
    ImGui::End();
}
