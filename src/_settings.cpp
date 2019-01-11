#include <stdio.h>
#include <stdlib.h>

typedef int camera_type_t;
typedef int camera_floor_t;
enum camera_type_ { VDB_CAMERA_DISABLED=0, VDB_CAMERA_PLANAR, VDB_CAMERA_TRACKBALL, VDB_CAMERA_TURNTABLE };
enum camera_floor_ { VDB_FLOOR_XY=0, VDB_FLOOR_XZ, VDB_FLOOR_YZ };
enum { MAX_FRAME_SETTINGS = 1024 };
enum { VDB_MAX_RENDER_SCALE_DOWN = 3 };
enum { VDB_MAX_RENDER_SCALE_UP = 3 };

static camera_type_t CameraTypeFromStr(const char *str)
{
    if (strcmp(str, "disabled") == 0) return VDB_CAMERA_DISABLED;
    else if (strcmp(str, "planar") == 0) return VDB_CAMERA_PLANAR;
    else if (strcmp(str, "trackball") == 0) return VDB_CAMERA_TRACKBALL;
    else if (strcmp(str, "turntable") == 0) return VDB_CAMERA_TURNTABLE;
    return VDB_CAMERA_DISABLED;
}

static const char *CameraTypeToStr(camera_type_t type)
{
    if (type == VDB_CAMERA_DISABLED) return "disabled";
    else if (type == VDB_CAMERA_PLANAR) return "planar";
    else if (type == VDB_CAMERA_TRACKBALL) return "trackball";
    else if (type == VDB_CAMERA_TURNTABLE) return "turntable";
    return "disabled";
}

static camera_floor_t CameraFloorFromStr(const char *str)
{
    if (strcmp(str, "xy") == 0) return VDB_FLOOR_XY;
    else if (strcmp(str, "xz") == 0) return VDB_FLOOR_XZ;
    else if (strcmp(str, "yz") == 0) return VDB_FLOOR_YZ;
    return VDB_FLOOR_XY;
}

static const char *CameraFloorToStr(camera_floor_t mode)
{
    if (mode == VDB_FLOOR_XY) return "xy";
    else if (mode == VDB_FLOOR_XZ) return "xz";
    else if (mode == VDB_FLOOR_YZ) return "yz";
    return "xy";
}

struct frame_settings_t
{
    char *name;
    camera_type_t camera_type;

    // camera initialization parameters
    float init_radius;
    vdbVec3 init_look_at;

    // perspective projection parameters
    float y_fov;
    float min_depth;
    float max_depth;

    camera_floor_t camera_floor;
    bool grid_visible;
    float grid_scale;
    bool cube_visible;

    int render_scale_down;
    int render_scale_up;
};

struct camera_settings_t
{
    float mouse_sensitivity;
    float scroll_sensitivity;
    float move_speed_normal;
    float move_speed_slow;

    // proportional smoothing gains
    float Kp_zoom;
    float Kp_translate;
    float Kp_rotate;
};

struct window_settings_t
{
    int x,y,width,height;
};

struct settings_t
{
    camera_settings_t camera;
    window_settings_t window;
    frame_settings_t frames[MAX_FRAME_SETTINGS];
    int num_frames;
    bool never_ask_on_exit;
    bool show_main_menu;
    int font_size;

    void LoadOrDefault(const char *filename);
    void Save(const char *filename);
};

static settings_t settings;

int ClampSetting(int x, int x_min, int x_max)
{
    if (x < x_min) return x_min;
    else if (x > x_max) return x_max;
    return x;
}

void DefaultFrameSettings(frame_settings_t *fs)
{
    fs->camera_type = VDB_CAMERA_DISABLED;
    fs->init_radius = 1.0f;
    fs->init_look_at = vdbVec3(0.0f, 0.0f, 0.0f);
    fs->y_fov = 0.7f;
    fs->min_depth = 0.1f;
    fs->max_depth = 50.0f;
    fs->grid_visible = false;
    fs->grid_scale = 2.0f;
    fs->camera_floor = VDB_FLOOR_XY;
    fs->cube_visible = false;
    fs->render_scale_down = 0;
    fs->render_scale_up = 0;
}

void settings_t::LoadOrDefault(const char *filename)
{
    camera.mouse_sensitivity = 50.0f;
    camera.scroll_sensitivity = 5.0f;
    camera.move_speed_normal = 1.0f;
    camera.move_speed_slow = 0.5f;
    camera.Kp_zoom = 5.0f;
    camera.Kp_translate = 5.0f;
    camera.Kp_rotate = 10.0f;
    window.x = -1;
    window.y = -1;
    window.width = 1000;
    window.height = 600;
    never_ask_on_exit = false;
    show_main_menu = true;
    num_frames = 0;
    font_size = (int)(VDB_DEFAULT_FONT_SIZE);

    FILE *f = fopen(filename, "rb");
    if (!f) return;
    if (fseek(f, 0, SEEK_END)) { fclose(f); return; }
    int file_size = (int)ftell(f);
    if (file_size <= 0)        { fclose(f); return; }
    if (fseek(f, 0, SEEK_SET)) { fclose(f); return; }

    frame_settings_t *frame = NULL;
    char *line = (char*)malloc(file_size);
    char *str = (char*)malloc(file_size);
    while (fgets(line, file_size, f))
    {
        int x,y;
        int i;
        float f;
        if      (sscanf(line, "window_pos=%d,%d", &x, &y) == 2)  { window.x = x; window.y = y; }
        else if (sscanf(line, "window_size=%d,%d", &x, &y) == 2) { window.width = x; window.height = y; }
        else if (sscanf(line, "never_ask_on_exit=%d", &i) == 1)  { never_ask_on_exit = (i != 0); }
        else if (sscanf(line, "show_main_menu=%d", &i) == 1)     { show_main_menu = (i != 0); }
        else if (sscanf(line, "mouse_sensitivity=%f", &f) == 1)  { camera.mouse_sensitivity = f; }
        else if (sscanf(line, "scroll_sensitivity=%f", &f) == 1) { camera.scroll_sensitivity = f; }
        else if (sscanf(line, "move_speed_normal=%f", &f) == 1)  { camera.move_speed_normal = f; }
        else if (sscanf(line, "move_speed_slow=%f", &f) == 1)    { camera.move_speed_slow = f; }
        else if (sscanf(line, "Kp_zoom=%f", &f) == 1)            { camera.Kp_zoom = f; }
        else if (sscanf(line, "Kp_translate=%f", &f) == 1)       { camera.Kp_translate = f; }
        else if (sscanf(line, "Kp_rotate=%f", &f) == 1)          { camera.Kp_rotate = f; }
        else if (sscanf(line, "font_size=%f", &f) == 1)          { font_size = ClampSetting((int)f, 6, 96); }
        else if (strstr(line, "[frame]=") == line)
        {
            if (num_frames == MAX_FRAME_SETTINGS)
            {
                frame = NULL;
                printf("vdb: Reached max number of stored per-block settings. You should clean up your vdb.ini file!\n");
                continue;
            }
            assert(num_frames < MAX_FRAME_SETTINGS);
            frame = frames + (num_frames++);
            frame->name = strdup(line + strlen("[frame]="));
            // remove trailing newlines:
            size_t len = strlen(frame->name);
            while (len > 0 && (frame->name[len-1] == '\n' || frame->name[len-1] == '\r'))
                frame->name[--len] = '\0';
            DefaultFrameSettings(frame);
        }
        else if (frame && sscanf(line, "camera_type=%s", str) == 1)      { frame->camera_type = CameraTypeFromStr(str); }
        else if (frame && sscanf(line, "camera_radius=%f", &f) == 1)     { frame->init_radius = f; }
        else if (frame && sscanf(line, "y_fov=%f", &f) == 1)             { frame->y_fov = f; }
        else if (frame && sscanf(line, "min_depth=%f", &f) == 1)         { frame->min_depth = f; }
        else if (frame && sscanf(line, "max_depth=%f", &f) == 1)         { frame->max_depth = f; }
        else if (frame && sscanf(line, "grid_visible=%d", &i) == 1)      { frame->grid_visible = (i == 1) ? true : false; }
        else if (frame && sscanf(line, "grid_scale=%f", &f) == 1)        { frame->grid_scale = f; }
        else if (frame && sscanf(line, "camera_floor=%s", str) == 1)     { frame->camera_floor = CameraFloorFromStr(str); }
        else if (frame && sscanf(line, "cube_visible=%d", &i) == 1)      { frame->cube_visible = (i == 1) ? true : false; }
        else if (frame && sscanf(line, "render_scale_down=%d", &i) == 1) { frame->render_scale_down = ClampSetting(i, 0, VDB_MAX_RENDER_SCALE_DOWN); }
        else if (frame && sscanf(line, "render_scale_up=%d", &i) == 1)   { frame->render_scale_up = ClampSetting(i, 0, VDB_MAX_RENDER_SCALE_UP); }
    }
    free(line);
    free(str);
    fclose(f);
}

void settings_t::Save(const char *filename)
{
    FILE *f = fopen(filename, "wb+");
    if (!f)
    {
        printf("Failed to save settings.\n");
        return;
    }
    fprintf(f, "[vdb]\n");
    fprintf(f, "window_pos=%d,%d\n", window.x, window.y);
    fprintf(f, "window_size=%d,%d\n", window.width, window.height);
    fprintf(f, "never_ask_on_exit=%d\n", never_ask_on_exit);
    fprintf(f, "show_main_menu=%d\n", show_main_menu);
    fprintf(f, "mouse_sensitivity=%g\n", camera.mouse_sensitivity);
    fprintf(f, "scroll_sensitivity=%g\n", camera.scroll_sensitivity);
    fprintf(f, "move_speed_normal=%g\n", camera.move_speed_normal);
    fprintf(f, "move_speed_slow=%g\n", camera.move_speed_slow);
    fprintf(f, "Kp_zoom=%g\n", camera.Kp_zoom);
    fprintf(f, "Kp_translate=%g\n", camera.Kp_translate);
    fprintf(f, "Kp_rotate=%g\n", camera.Kp_rotate);
    fprintf(f, "font_size=%d\n", font_size);
    for (int i = 0; i < num_frames; i++)
    {
        frame_settings_t *frame = frames + i;
        fprintf(f, "\n[frame]=%s\n", frame->name);
        if (frame->camera_type != VDB_CAMERA_DISABLED)
        {
            fprintf(f, "camera_type=%s\n", CameraTypeToStr(frame->camera_type));
            fprintf(f, "camera_radius=%g\n", frame->init_radius);
            if (frame->camera_type != VDB_CAMERA_PLANAR)
            {
                fprintf(f, "y_fov=%g\n", frame->y_fov);
                fprintf(f, "min_depth=%g\n", frame->min_depth);
                fprintf(f, "max_depth=%g\n", frame->max_depth);
            }
            fprintf(f, "grid_visible=%d\n", frame->grid_visible ? 1 : 0);
            fprintf(f, "grid_scale=%g\n", frame->grid_scale);
            fprintf(f, "camera_floor=%s\n", CameraFloorToStr(frame->camera_floor));
            fprintf(f, "cube_visible=%d\n", frame->cube_visible ? 1 : 0);
            fprintf(f, "render_scale_down=%d\n", frame->render_scale_down);
            fprintf(f, "render_scale_up=%d\n", frame->render_scale_up);
        }
    }
    fclose(f);
}
