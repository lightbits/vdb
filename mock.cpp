#define SO_PLATFORM_IMPLEMENTATION
#define SO_PLATFORM_IMGUI
#include "lib/imgui/imgui_draw.cpp"
#include "lib/imgui/imgui.cpp"
#include "lib/imgui/imgui_demo.cpp"
#include "lib/so_platform.h"

struct vdb_state
{
    const char *label;
};

vdb_state vdb_init(const char *label)
{
    static bool have_window = false;
    if (!have_window)
    {
        so_openWindow("vdb", 640, 480, -1, -1, 1, 5, 0, 32, 8, 24, 0);
        so_imgui_init();
    }
    vdb_state result = {0};
    result.label = label;
    return result;
}

void vdb_preamble(so_input input, vdb_state *state)
{
    so_imgui_processEvents(input);

    glViewport(0, 0, input.width, input.height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::NewFrame();
}

void vdb_postamble(so_input input, vdb_state *state)
{
    ImGui::Render();
    so_swapBuffers();
}

#define VDBB(LABEL) vdb_state vdb_##__LINE__##_state = vdb_init(LABEL);     \
                    so_input vdb_##__LINE__##_input = {0};                  \
                    while (so_loopWindow(&vdb_##__LINE__##_input)) {        \
                    using namespace ImGui;                                  \
                    vdb_preamble(vdb_##__LINE__##_input, &vdb_##__LINE__##_state);

#define VDBE()      vdb_postamble(vdb_##__LINE__##_input, &vdb_##__LINE__##_state); }

int main()
{
    VDBB("my label");
    {
        Begin("Test");
        ShowTestWindow();
        End();
    }
    VDBE();

    // so_openWindow("vdb", 640, 480, -1, -1, 1, 5, 0, 32, 8, 24, 0);
    // so_imgui_init();
    // so_input input = {0};
    // while (so_loopWindow(&input))
    // {
    //     so_imgui_processEvents(input);

    //     glViewport(0, 0, input.width, input.height);
    //     glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //     glClear(GL_COLOR_BUFFER_BIT);

    //     ImGui::NewFrame();

    //     // user code here

    //     ImGui::Render();
    //     so_swapBuffers();
    // }
    // so_imgui_shutdown();
    // so_closeWindow();
}
