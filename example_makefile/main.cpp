#include <vdb.h>
#include <vdb/imgui.h>

int main(int, char**)
{
    VDBB("Test");
    {
        ImGui::ShowTestWindow();
    }
    VDBE();
    return 0;
}
