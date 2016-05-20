#include "vdb.h"
#include "lib/imgui_draw.cpp"
#include "lib/imgui.cpp"
#include "lib/imgui_demo.cpp"

struct vdb_event
{
    bool StepOver;
    bool StepOnce;
    bool Exit;
    // bool TakeScreenshot;
    // bool Take;
};

enum vdb_keys;

#ifdef _WIN32
#include "vdb_win32.cpp"
#undef GetWindowFont
#else
#error "Your platform is not supported"
#endif

void vdb_imgui_renderDrawLists(ImDrawData* draw_data)
{
    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

    // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render command lists
    #define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
        const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
    #undef OFFSETOF

    // Restore modified state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}

void vdb_imgui_init()
{
    ImGuiIO &IO = ImGui::GetIO();
    IO.KeyMap[ImGuiKey_Tab] = VDB_KEY_TAB;
    IO.KeyMap[ImGuiKey_LeftArrow] = VDB_KEY_LEFT;
    IO.KeyMap[ImGuiKey_RightArrow] = VDB_KEY_RIGHT;
    IO.KeyMap[ImGuiKey_UpArrow] = VDB_KEY_UP;
    IO.KeyMap[ImGuiKey_DownArrow] = VDB_KEY_DOWN;
    IO.KeyMap[ImGuiKey_PageUp] = VDB_KEY_PAGEUP;
    IO.KeyMap[ImGuiKey_PageDown] = VDB_KEY_PAGEDOWN;
    IO.KeyMap[ImGuiKey_Home] = VDB_KEY_HOME;
    IO.KeyMap[ImGuiKey_End] = VDB_KEY_END;
    IO.KeyMap[ImGuiKey_Delete] = VDB_KEY_DELETE;
    IO.KeyMap[ImGuiKey_Backspace] = VDB_KEY_BACK;
    IO.KeyMap[ImGuiKey_Enter] = VDB_KEY_ENTER;
    IO.KeyMap[ImGuiKey_Escape] = VDB_KEY_ESCAPE;
    IO.KeyMap[ImGuiKey_A] = VDB_KEY_A;
    IO.KeyMap[ImGuiKey_C] = VDB_KEY_C;
    IO.KeyMap[ImGuiKey_V] = VDB_KEY_V;
    IO.KeyMap[ImGuiKey_X] = VDB_KEY_X;
    IO.KeyMap[ImGuiKey_Y] = VDB_KEY_Y;
    IO.KeyMap[ImGuiKey_Z] = VDB_KEY_Z;

    IO.RenderDrawListsFn = vdb_imgui_renderDrawLists;   // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    IO.SetClipboardTextFn = vdb_setClipboardText;
    IO.GetClipboardTextFn = vdb_getClipboardText;

    #ifdef VDB_MY_CONFIG
    ImGuiStyle &Style = ImGui::GetStyle();
    Style.FrameRounding = 2.0f;
    Style.GrabRounding = 2.0f;
    IO.IniFilename = "./build/imgui.ini";
    IO.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/SourceSansPro-SemiBold.ttf", 18.0f);
    #endif

    // Build texture atlas
    unsigned char* Pixels;
    int Width, Height;
    IO.Fonts->GetTexDataAsAlpha8(&Pixels, &Width, &Height);

    // Upload texture to graphics system
    GLint LastTexture;
    GLuint FontTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &LastTexture);
    glGenTextures(1, &FontTexture);
    glBindTexture(GL_TEXTURE_2D, FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, Width, Height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, Pixels);

    // Store our identifier
    IO.Fonts->TexID = (void *)(intptr_t)FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, LastTexture);
}

void vdb(char *Label, vdb_callback Callback)
{
    static char *PrevLabel = 0;
    static bool Initialized = false;
    static bool StepOver = false;
    static vdb_window Window = {0};
    if (!Initialized)
    {
        Window = vdb_openWindow(640, 480);
        vdb_imgui_init();
        Initialized = true;
    }

    // @ maybe store the callback addresses?
    // recall

    bool IsNewCallback = false;
    if (PrevLabel != Label)
    {
        PrevLabel = Label;
        IsNewCallback = true;
    }

    if (StepOver)
    {
        if (IsNewCallback)
        {
            StepOver = false;
        }
        else
        {
            return;
        }
    }

    float MainGuiAlpha = 1.0f;
    vdb_input Input = {0};
    vdb_event Event = {0};
    while (vdb_processMessages(Window, &Input, &Event))
    {
        ImGui::NewFrame();

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, MainGuiAlpha);
        Callback(Input);
        ImGui::PopStyleVar();

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.7f);
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                if (ImGui::SliderFloat("Gui alpha", &MainGuiAlpha, 0.0f, 1.0f))
                {
                    if (MainGuiAlpha < 0.05f)
                        MainGuiAlpha = 0.0f;
                    if (MainGuiAlpha > 0.95f)
                        MainGuiAlpha = 1.0f;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debugger"))
            {
                if (ImGui::MenuItem("Step once"))
                    Event.StepOnce = true;
                if (ImGui::MenuItem("Step over"))
                    Event.StepOver = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar();

        if (Event.StepOnce)
        {
            break;
        }
        if (Event.StepOver)
        {
            StepOver = true;
            break;
        }

        ImGui::Render();

        vdb_swapBuffers(Window);
        vdb_sleep(4); // @ performance counters, fps lock
    }

    if (Event.Exit)
    {
        vdb_exit();
    }
}
