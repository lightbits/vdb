#include "data/window_icon.h"

void PostGLCallback(const char *name, void *funcptr, int len_args, ...) {
    (void) funcptr;
    (void) len_args;

    GLenum error_code = glad_glGetError();

    if (error_code != GL_NO_ERROR) {
        fprintf(stderr, "ERROR %d (0x%x) in %s\n", error_code, error_code, name);
        assert(false && "Error after OpenGL function call. Run with a debugger to see where and why it crashed.");
    }
}

namespace window
{
    static bool vsynced;
    static SDL_Window *sdl_window;
    static SDL_GLContext sdl_gl_context;

    // Note: for retina displays screen coordinates != framebuffer coordinates
    // This is the size in pixels of the framebuffer in the window
    static int framebuffer_width;
    static int framebuffer_height;
    // This is the size of the window's client area in screen coordinates
    static int window_width;
    static int window_height;
    // This is the position of the window's client area in screen coordinates
    static int window_x;
    static int window_y;

    static bool should_quit;

    static bool dont_wait_next_frame_events;

    static void DetachGLContext()
    {
        assert(sdl_window);
        SDL_GL_MakeCurrent(sdl_window, NULL);
    }

    static void EnsureGLContextIsCurrent()
    {
        if (SDL_GL_GetCurrentContext() != sdl_gl_context)
            SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);
    }

    static void Close()
    {
        SDL_GL_DeleteContext(sdl_gl_context);
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
    }

    static void SetSize(int width, int height, bool topmost)
    {
        #ifdef _WIN32
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        if (SDL_GetWindowWMInfo(sdl_window, &info))
        {
            HWND hwnd = info.info.win.window;
            RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.right = width;
            rect.bottom = height;
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
            SDL_SetWindowSize(sdl_window, width, height);
        }
        #else
        SDL_SetWindowSize(sdl_window, width, height);
        #endif
    }

    static void Open(int x, int y, int width, int height)
    {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        {
            printf("SDL_Init failed: %s\n", SDL_GetError());
            assert(false);
            exit(EXIT_FAILURE);
        }

        #if __APPLE__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1); // todo: have not tested on OSX!
        #else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        #endif

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, VDB_ALPHABITS);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, VDB_DEPTHBITS);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, VDB_STENCILBITS);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, VDB_MULTISAMPLES > 0 ? 1 : 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, VDB_MULTISAMPLES);

        sdl_window = SDL_CreateWindow(
            "vdb",
            (x < 0) ? SDL_WINDOWPOS_CENTERED : x,
            (y < 0) ? SDL_WINDOWPOS_CENTERED : y,
            width,
            height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

        assert(sdl_window);

        // Set window icon
        {
            // todo: do we need to change these masks depending on endian-ness?
            Uint32 rm=0x000000ff,
                   gm=0x0000ff00,
                   bm=0x00ff0000,
                   am=0xff000000;
            int x,y,n;
            stbi_uc *pixels = stbi_load_from_memory((const stbi_uc*)window_icon_data, window_icon_size, &x, &y, &n, 4);
            assert(pixels);
            SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(pixels, x, y, 32, x*4, rm, gm, bm, am);
            if (icon)
                SDL_SetWindowIcon(sdl_window, icon);
        }

        SDL_ShowWindow(sdl_window);

        SDL_GL_LoadLibrary(NULL); // GLAD will do the loading for us after creating context
        sdl_gl_context = SDL_GL_CreateContext(sdl_window);
        assert(sdl_gl_context != 0);
        assert(gladLoadGLLoader(SDL_GL_GetProcAddress));
        assert(gladLoadGL());

        #ifdef VDB_DEBUG
        glad_set_post_callback(PostGLCallback);
        #endif

        // 0 for immediate updates, 1 for updates synchronized with the
        // vertical retrace. If the system supports it, you may
        // specify -1 to allow late swaps to happen immediately
        // instead of waiting for the next retrace.
        SDL_GL_SetSwapInterval(1);

        // Instead of using vsync, you can specify a desired framerate
        // that the application will attempt to keep. If a frame rendered
        // too fast, it will sleep the remaining time. Leave swap_interval
        // at 0 when using this.
        vsynced = (SDL_GL_GetSwapInterval() == 1);

        // printf("Vendor        : %s\n", glGetString(GL_VENDOR));
        // printf("Renderer      : %s\n", glGetString(GL_RENDERER));
        // printf("Version       : %s\n", glGetString(GL_VERSION));
        // printf("Extensions    : %s\n", glGetString(GL_EXTENSIONS));
    }

    static void SwapBuffers(float dt)
    {
        SDL_GL_SwapWindow(sdl_window);
        if (!vsynced && dt < 1.0f/60.0f)
        {
            float sleep_s = (1.0f/60.0f) - dt;
            Uint32 sleep_ms = (Uint32)(sleep_s*1000.0f);
            SDL_Delay(sleep_ms);
        }
    }

    static void BeforeEvents()
    {
        mouse::wheel = 0.0f;
        for (int i = 0; i < SDL_NUM_SCANCODES; i++)
        {
            keys::pressed[i] = false;
            keys::released[i] = false;
        }
        mouse::left.pressed = false;
        mouse::right.pressed = false;
        mouse::middle.pressed = false;
        mouse::left.released = false;
        mouse::right.released = false;
        mouse::middle.released = false;
    }

    static void AfterEvents()
    {
        SDL_GL_GetDrawableSize(sdl_window, &framebuffer_width, &framebuffer_height);
        SDL_GetWindowPosition(sdl_window, &window_x, &window_y);
        SDL_GetMouseState(&mouse::x, &mouse::y);
        SDL_GetWindowSize(sdl_window, &window_width, &window_height);

        settings.window.width = window_width;
        settings.window.height = window_height;
        settings.window.x = window_x;
        settings.window.y = window_y;
    }

    static void ProcessEvent(const SDL_Event *event)
    {
        ImGui_ImplSDL2_ProcessEvent(event);
        if (event->type == SDL_QUIT)
        {
            should_quit = true;
        }
        else
        if (event->type == SDL_KEYDOWN)
        {
            SDL_Scancode c = event->key.keysym.scancode;
            if (!keys::down[c])
                keys::pressed[c] = true;
            keys::down[c] = true;
        }
        else
        if (event->type == SDL_KEYUP)
        {
            SDL_Scancode c = event->key.keysym.scancode;
            if (keys::down[c])
                keys::released[c] = true;
            keys::down[c] = false;
        }
        else
        if (event->type == SDL_MOUSEBUTTONDOWN)
        {
            if (event->button.button == SDL_BUTTON_LEFT) { if (!mouse::left.down) mouse::left.pressed = true; mouse::left.down = true; }
            if (event->button.button == SDL_BUTTON_RIGHT) { if (!mouse::right.down) mouse::right.pressed = true; mouse::right.down = true; }
            if (event->button.button == SDL_BUTTON_MIDDLE) { if (!mouse::middle.down) mouse::middle.pressed = true; mouse::middle.down = true; }
        }
        else
        if (event->type == SDL_MOUSEBUTTONUP)
        {
            if (event->button.button == SDL_BUTTON_LEFT) { if (mouse::left.down) mouse::left.released = true; mouse::left.down = false; }
            if (event->button.button == SDL_BUTTON_RIGHT) { if (mouse::right.down) mouse::right.released = true; mouse::right.down = false; }
            if (event->button.button == SDL_BUTTON_MIDDLE) { if (mouse::middle.down) mouse::middle.released = true; mouse::middle.down = false; }
        }
        else
        if (event->type == SDL_MOUSEWHEEL)
        {
            if (event->wheel.y > 0) mouse::wheel = +1.0f;
            else if (event->wheel.y < 0) mouse::wheel = -1.0f;
        }
    }

    static void DontWaitNextFrameEvents()
    {
        dont_wait_next_frame_events = true;
    }

    static void PollEvents()
    {
        BeforeEvents();
        SDL_Event event;
        while (SDL_PollEvent(&event))
            ProcessEvent(&event);
        AfterEvents();
    }

    static int settle_frames = 0;
    static void SetNumSettleFrames(int frames)
    {
        settle_frames = frames;
    }

    static void SetMinimumNumSettleFrames(int frames)
    {
        if (frames > settle_frames)
            settle_frames = frames;
    }

    static void WaitEvents()
    {
        static int idle_frames = 0;
        if (dont_wait_next_frame_events || idle_frames < settle_frames)
        {
            idle_frames++;
            dont_wait_next_frame_events = false;

            BeforeEvents();
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ProcessEvent(&event);
                idle_frames = 0;
            }
            AfterEvents();
        }
        else
        {
            BeforeEvents();
            SDL_Event event;
            SDL_WaitEvent(&event);
            do
            {
                ProcessEvent(&event);
            } while (SDL_PollEvent(&event));
            AfterEvents();
            idle_frames = 0;
        }
    }
}
