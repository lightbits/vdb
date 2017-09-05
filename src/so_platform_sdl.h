#ifndef SO_PLATFORM_HEADER_INCLUDE
#define SO_PLATFORM_HEADER_INCLUDE
#define SDL_ASSERT_LEVEL 2
#include <SDL.h>
#include <SDL_assert.h>
#include <SDL_opengl.h>
#define SO_PLATFORM_KEY(KEY) SDL_SCANCODE_##KEY

typedef struct
{
    int x, y; // Pixel coordinate from top-left
    float u, v; // [-1, +1] normalized coordinate from bottom-left
    int wheel;
} so_input_mouse;

typedef struct
{
    bool down; // Is down?
    bool released; // Was there a transition from down to up since last check?
    bool pressed; // Was there a transition from up to down since last check?
} so_input_button;

struct so_input
{
    so_input_mouse mouse;
    so_input_button left; // Left mouse button
    so_input_button middle; // Middle mouse button
    so_input_button right; // Right mouse button
    so_input_button *keys; // Array of key states accessible through SO_KEY_* enums
    int shift, ctrl, alt; // Key modifiers (1 if active)
    int win_x, win_y; // Window upper-left corner relative upper-left corner of screen
    int width, height; // Window width and height in pixels
    float dt; // Seconds elapsed since previous so_loopWindow was called
    float t; // Seconds elapsed since so_openWindow was called

    char *text; // utf-8 null-terminated text input
};

void so_openWindow(
    const char *title,
    int width,
    int height,
    int x = -1,
    int y = -1,
    int major = 3,
    int minor = 1,
    int multisamples = 4,
    int alpha_bits = 8,
    int depth_bits = 24,
    int stencil_bits = 0);
bool so_loopWindow(so_input *input);
void so_closeWindow();
void so_setWindowSize(int w, int h, bool topmost);
void so_swapBuffersAndSleep(float dt);
#endif

#ifdef SO_PLATFORM_IMPLEMENTATION
static struct so_platform
{
    SDL_Window *window;
    SDL_GLContext gl;
    bool has_vsync;
    GLuint font_texture;
} so_platform_;

void so_swapBuffersAndSleep(float dt)
{
    SDL_GL_SwapWindow(so_platform_.window);
    if (!so_platform_.has_vsync && dt < 1.0f/60.0f)
    {
        float sleep_s = (1.0f/60.0f) - dt;
        Uint32 sleep_ms = (Uint32)(sleep_s*1000.0f);
        SDL_Delay(sleep_ms);
    }
}

#ifdef _WIN32
#include <winuser.h>
#include <SDL_syswm.h>
#endif

void so_setWindowSize(int w, int h, bool topmost)
{
    #ifdef _WIN32
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(so_platform_.window, &info))
    {
        HWND hwnd = info.info.win.window;
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = w;
        rect.bottom = h;
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
        int aw = rect.right-rect.left;
        int ah = rect.bottom-rect.top;
        if (topmost)
        {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, aw, ah, SWP_NOMOVE);
        }
        else
        {
            SetWindowPos(hwnd, HWND_TOP, 0, 0, aw, ah, SWP_NOMOVE);
        }
    }
    else
    {
        SDL_SetWindowSize(so_platform_.window, w, h);
    }
    #else
    SDL_SetWindowSize(so_platform_.window, w, h);
    #endif
}

void so_closeWindow()
{
    SDL_GL_DeleteContext(so_platform_.gl);
    SDL_DestroyWindow(so_platform_.window);
}

static const char* so_getClipboardUTF8()
{
    return SDL_GetClipboardText();
}

static void so_setClipboardUTF8(const char* text)
{
    SDL_SetClipboardText(text);
}

void so_openWindow(
    const char *title,
    int width,
    int height,
    int x,
    int y,
    int major,
    int minor,
    int multisamples,
    int alpha_bits,
    int depth_bits,
    int stencil_bits)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        SDL_assert(false);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, alpha_bits);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depth_bits);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencil_bits);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, multisamples > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisamples);

    so_platform_.window = SDL_CreateWindow(
        title,
        (x < 0) ? SDL_WINDOWPOS_CENTERED : x,
        (y < 0) ? SDL_WINDOWPOS_CENTERED : y,
        width,
        height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_assert(so_platform_.window);

    so_platform_.gl = SDL_GL_CreateContext(so_platform_.window);
    SDL_assert(so_platform_.gl != 0);

    // 0 for immediate updates, 1 for updates synchronized with the
    // vertical retrace. If the system supports it, you may
    // specify -1 to allow late swaps to happen immediately
    // instead of waiting for the next retrace.
    SDL_GL_SetSwapInterval(1);

    // Instead of using vsync, you can specify a desired framerate
    // that the application will attempt to keep. If a frame rendered
    // too fast, it will sleep the remaining time. Leave swap_interval
    // at 0 when using this.
    if (SDL_GL_GetSwapInterval() == 1)
        so_platform_.has_vsync = true;
    else
        so_platform_.has_vsync = false;
}

bool so_loopWindow(so_input *input)
{
    static so_input_button keys[SDL_NUM_SCANCODES] = {0};
    input->keys = keys;

    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        keys[i].pressed = false;
        keys[i].released = false;
    }

    input->left.released = false;
    input->left.pressed = false;
    input->middle.released = false;
    input->middle.pressed = false;
    input->right.released = false;
    input->right.pressed = false;

    input->mouse.wheel = 0;

    static char text_utf8[1024];
    int cur_text_utf8 = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            {
                return false;
            } break;

            case SDL_KEYDOWN:
            {
                SDL_Scancode c = event.key.keysym.scancode;
                if (!keys[c].down)
                    keys[c].pressed = true;
                keys[c].down = true;

                if (c == SDL_SCANCODE_LALT) input->alt = true;
                if (c == SDL_SCANCODE_LCTRL) input->ctrl = true;
                if (c == SDL_SCANCODE_LSHIFT) input->shift = true;
            } break;

            case SDL_KEYUP:
            {
                SDL_Scancode c = event.key.keysym.scancode;
                if (keys[c].down)
                    keys[c].released = true;
                keys[c].down = false;

                if (c == SDL_SCANCODE_LALT) input->alt = false;
                if (c == SDL_SCANCODE_LCTRL) input->ctrl = false;
                if (c == SDL_SCANCODE_LSHIFT) input->shift = false;
            } break;

            #define so__btn_up(btn) { if ((btn).down) btn.released = true; btn.down = false; }
            #define so__btn_down(btn) { if (!(btn).down) btn.pressed = true; btn.down = true; }

            case SDL_MOUSEBUTTONDOWN:
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                    so__btn_down(input->left);
                if (event.button.button == SDL_BUTTON_RIGHT)
                    so__btn_down(input->right);
                if (event.button.button == SDL_BUTTON_MIDDLE)
                    so__btn_down(input->middle);
            } break;

            case SDL_MOUSEBUTTONUP:
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                    so__btn_up(input->left);
                if (event.button.button == SDL_BUTTON_RIGHT)
                    so__btn_up(input->right);
                if (event.button.button == SDL_BUTTON_MIDDLE)
                    so__btn_up(input->middle);
            } break;

            case SDL_MOUSEWHEEL:
            {
                if (event.wheel.y > 0)
                    input->mouse.wheel = +1;
                else if (event.wheel.y < 0)
                    input->mouse.wheel = -1;
            } break;

            case SDL_TEXTINPUT:
            {
                int len = 0;
                char *c = event.text.text;
                while (*c)
                {
                    c++;
                    len++;
                }

                if (cur_text_utf8 + len + 1 <= (int)sizeof(text_utf8))
                {
                    for (int i = 0; i < len; i++)
                    {
                        text_utf8[cur_text_utf8++] = event.text.text[i];
                    }
                }
            } break;
        }
    }

    {
        #if 0
        SDL_GL_GetDrawableSize(so_platform_.window, &input->width, &input->height);
        #else
        SDL_GetWindowSize(so_platform_.window, &input->width, &input->height);
        #endif
    }

    {
        SDL_GetWindowPosition(so_platform_.window, &input->win_x, &input->win_y);
    }

    {
        SDL_GetMouseState(&input->mouse.x, &input->mouse.y);
        int w, h;
        SDL_GetWindowSize(so_platform_.window, &w, &h);
        input->mouse.u = -1.0f + 2.0f*input->mouse.x/w;
        input->mouse.v = -1.0f + 2.0f*(h-input->mouse.y-1)/h;
    }

    {
        input->text = (cur_text_utf8 > 0) ? text_utf8 : 0;
    }

    {
        Uint64 count = SDL_GetPerformanceCounter();
        Uint64 frequency = SDL_GetPerformanceFrequency();

        static Uint64 prev_count = 0;
        static Uint64 init_count = 0;
        if (!init_count)
        {
            init_count = count;
            prev_count = count;
        }

        input->dt = (float)(count - prev_count) / (float)frequency;
        input->t = (float)(count - init_count) / (float)frequency;

        prev_count = count;
    }

    return true;
}
#endif

#ifdef SO_PLATFORM_IMGUI
void so__imgui_drawLists(ImDrawData* draw_data)
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

void so_imgui_init()
{
    ImGuiIO& io = ImGui::GetIO();

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

    io.RenderDrawListsFn = so__imgui_drawLists;
    io.SetClipboardTextFn = so_setClipboardUTF8;
    io.GetClipboardTextFn = so_getClipboardUTF8;

    ImGuiStyle &style = ImGui::GetStyle();
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    {
        style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.26f, 0.38f, 0.70f);
        style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.20f, 0.35f, 0.47f, 0.70f);
        style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.18f, 0.18f, 0.18f, 0.39f);
        style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.18f, 0.18f, 0.18f, 0.55f);
        style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
        style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.42f);
        style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.24f, 0.24f, 0.24f, 0.59f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.31f, 0.31f, 0.31f, 0.59f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.78f, 0.78f, 0.78f, 0.40f);
        style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.24f, 0.52f, 0.88f, 0.90f);
        style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 0.90f);
        style.Colors[ImGuiCol_Button]                = ImVec4(0.16f, 0.26f, 0.38f, 0.78f);
        style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.18f, 0.28f, 0.40f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.20f, 0.31f, 0.47f, 1.00f);
        style.Colors[ImGuiCol_Header]                = ImVec4(0.16f, 0.26f, 0.38f, 0.80f);
        style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.24f, 0.52f, 0.88f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.24f, 0.52f, 0.88f, 0.80f);
        style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.16f, 0.26f, 0.38f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.16f, 0.26f, 0.38f, 0.90f);
        style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.20f, 0.29f, 0.43f, 0.90f);
        style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.29f, 0.29f, 0.29f, 0.50f);
        style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.39f, 0.39f, 0.39f, 0.60f);


    }
    #ifdef VDB_FONT
    io.Fonts->AddFontFromFileTTF(VDB_FONT);
    #endif
    io.IniFilename = VDB_IMGUI_INI_FILENAME;
    io.IniSavingRate = 1.0f;

    // Build texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &so_platform_.font_texture);
    glBindTexture(GL_TEXTURE_2D, so_platform_.font_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)so_platform_.font_texture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);
}

void so_imgui_shutdown()
{
    glDeleteTextures(1, &so_platform_.font_texture);
    ImGui::GetIO().Fonts->TexID = 0;
    so_platform_.font_texture = 0;
    ImGui::Shutdown();
}

void so_imgui_processEvents(so_input input)
{
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = input.dt > 0.0f ? input.dt : 1.0f / 60.0f;
    io.DisplaySize.x = (float)input.width;
    io.DisplaySize.y = (float)input.height;

    io.MousePos.x = (float)input.mouse.x;
    io.MousePos.y = (float)input.mouse.y;

    io.MouseWheel = (float)input.mouse.wheel;

    io.MouseDown[0] = input.left.pressed || input.left.down;
    io.MouseDown[1] = input.right.pressed || input.right.down;
    io.MouseDown[2] = input.middle.pressed || input.middle.down;

    if (input.text)
        io.AddInputCharactersUTF8(input.text);

    for (int key = 0; key < SDL_NUM_SCANCODES; key++)
        io.KeysDown[key] = input.keys[key].down;

    io.KeyShift = input.shift;
    io.KeyCtrl = input.ctrl;
    io.KeyAlt = input.alt;
}
#endif
