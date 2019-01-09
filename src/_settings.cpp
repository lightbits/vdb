#include <stdio.h>
#include <stdlib.h>

typedef int camera_type_t;
typedef int camera_floor_t;
enum camera_type_ { VDB_CAMERA_USER=0, VDB_CAMERA_2D, VDB_CAMERA_TRACKBALL, VDB_CAMERA_TURNTABLE };
enum camera_floor_ { VDB_FLOOR_XZ=0, VDB_FLOOR_XY, VDB_FLOOR_YZ };
enum { MAX_FRAME_SETTINGS = 1024 };

static camera_type_t CameraTypeFromStr(const char *str)
{
    if (strcmp(str, "User") == 0) return VDB_CAMERA_USER;
    else if (strcmp(str, "2D") == 0) return VDB_CAMERA_2D;
    else if (strcmp(str, "Trackball") == 0) return VDB_CAMERA_TRACKBALL;
    else if (strcmp(str, "Turntable") == 0) return VDB_CAMERA_TURNTABLE;
    return VDB_CAMERA_2D;
}

static const char *CameraTypeToStr(camera_type_t type)
{
    if (type == VDB_CAMERA_USER) return "User";
    else if (type == VDB_CAMERA_2D) return "2D";
    else if (type == VDB_CAMERA_TRACKBALL) return "Trackball";
    else if (type == VDB_CAMERA_TURNTABLE) return "Turntable";
    return "2D";
}

static camera_floor_t CameraFloorFromStr(const char *str)
{
    if (strcmp(str, "XY") == 0) return VDB_FLOOR_XY;
    else if (strcmp(str, "XZ") == 0) return VDB_FLOOR_XZ;
    else if (strcmp(str, "YZ") == 0) return VDB_FLOOR_YZ;
    return VDB_FLOOR_XZ;
}

static const char *CameraFloorToStr(camera_floor_t mode)
{
    if (mode == VDB_FLOOR_XY) return "XY";
    else if (mode == VDB_FLOOR_XZ) return "XZ";
    else if (mode == VDB_FLOOR_YZ) return "YZ";
    return "XZ";
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
    float cube_scale;
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

    void LoadOrDefault(const char *filename);
    void Save(const char *filename);
};

static settings_t settings;

void DefaultFrameSettings(frame_settings_t *fs)
{
    fs->camera_type = VDB_CAMERA_USER;
    fs->init_radius = 1.0f;
    fs->init_look_at = vdbVec3(0.0f, 0.0f, 0.0f);
    fs->y_fov = 0.7f;
    fs->min_depth = 0.1f;
    fs->max_depth = 50.0f;
    fs->grid_visible = false;
    fs->grid_scale = 10.0f;
    fs->camera_floor = VDB_FLOOR_XZ;
    fs->cube_visible = false;
    fs->cube_scale = 1.0f;
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
        if (sscanf(line, "Pos=%d,%d", &x, &y) == 2)             { window.x = x; window.y = y; }
        else if (sscanf(line, "Size=%d,%d", &x, &y) == 2)       { window.width = x; window.height = y; }
        else if (sscanf(line, "NeverAskOnExit=%d", &i) == 1)    { never_ask_on_exit = (i != 0); }
        else if (sscanf(line, "ShowMainMenu=%d", &i) == 1)      { show_main_menu = (i != 0); }
        else if (sscanf(line, "MouseSensitivity=%f", &f) == 1)  { camera.mouse_sensitivity = f; }
        else if (sscanf(line, "ScrollSensitivity=%f", &f) == 1) { camera.scroll_sensitivity = f; }
        else if (sscanf(line, "MoveSpeedNormal=%f", &f) == 1)   { camera.move_speed_normal = f; }
        else if (sscanf(line, "MoveSpeedSlow=%f", &f) == 1)     { camera.move_speed_slow = f; }
        else if (sscanf(line, "Kp_zoom=%f", &f) == 1)           { camera.Kp_zoom = f; }
        else if (sscanf(line, "Kp_translate=%f", &f) == 1)      { camera.Kp_translate = f; }
        else if (sscanf(line, "Kp_rotate=%f", &f) == 1)         { camera.Kp_rotate = f; }
        else if (strstr(line, "[Frame]=") == line)
        {
            if (num_frames == MAX_FRAME_SETTINGS)
            {
                frame = NULL;
                printf("vdb: Reached max number of stored per-block settings. You should clean up your vdb.ini file!\n");
                continue;
            }
            assert(num_frames < MAX_FRAME_SETTINGS);
            frame = frames + (num_frames++);
            frame->name = strdup(line + strlen("[Frame]="));
            // remove trailing newlines:
            size_t len = strlen(frame->name);
            while (len > 0 && (frame->name[len-1] == '\n' || frame->name[len-1] == '\r'))
                frame->name[--len] = '\0';
            DefaultFrameSettings(frame);
        }
        else if (frame && sscanf(line, "Camera=%s", str) == 1)      { frame->camera_type = CameraTypeFromStr(str); }
        else if (frame && sscanf(line, "CameraRadius=%f", &f) == 1) { frame->init_radius = f; }
        else if (frame && sscanf(line, "YFov=%f", &f) == 1)         { frame->y_fov = f; }
        else if (frame && sscanf(line, "MinDepth=%f", &f) == 1)     { frame->min_depth = f; }
        else if (frame && sscanf(line, "MaxDepth=%f", &f) == 1)     { frame->max_depth = f; }
        else if (frame && sscanf(line, "GridVisible=%d", &i) == 1)  { frame->grid_visible = (i == 1) ? true : false; }
        else if (frame && sscanf(line, "GridScale=%f", &f) == 1)    { frame->grid_scale = f; }
        else if (frame && sscanf(line, "Floor=%s", str) == 1)       { frame->camera_floor = CameraFloorFromStr(str); }
        else if (frame && sscanf(line, "CubeVisible=%d", &i) == 1)  { frame->cube_visible = (i == 1) ? true : false; }
        else if (frame && sscanf(line, "CubeScale=%f", &f) == 1)    { frame->cube_scale = f; }
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
    fprintf(f, "Pos=%d,%d\n", window.x, window.y);
    fprintf(f, "Size=%d,%d\n", window.width, window.height);
    fprintf(f, "NeverAskOnExit=%d\n", never_ask_on_exit);
    fprintf(f, "ShowMainMenu=%d\n", show_main_menu);
    fprintf(f, "MouseSensitivity=%f\n", camera.mouse_sensitivity);
    fprintf(f, "ScrollSensitivity=%f\n", camera.scroll_sensitivity);
    fprintf(f, "MoveSpeedNormal=%f\n", camera.move_speed_normal);
    fprintf(f, "MoveSpeedSlow=%f\n", camera.move_speed_slow);
    fprintf(f, "Kp_zoom=%f\n", camera.Kp_zoom);
    fprintf(f, "Kp_translate=%f\n", camera.Kp_translate);
    fprintf(f, "Kp_rotate=%f\n", camera.Kp_rotate);
    for (int i = 0; i < num_frames; i++)
    {
        frame_settings_t *frame = frames + i;
        fprintf(f, "\n[Frame]=%s\n", frame->name);
        if (frame->camera_type != VDB_CAMERA_USER)
        {
            fprintf(f, "Camera=%s\n", CameraTypeToStr(frame->camera_type));
            fprintf(f, "CameraRadius=%f\n", frame->init_radius);
            if (frame->camera_type != VDB_CAMERA_2D)
            {
                fprintf(f, "YFov=%f\n", frame->y_fov);
                fprintf(f, "MinDepth=%f\n", frame->min_depth);
                fprintf(f, "MaxDepth=%f\n", frame->max_depth);
            }
            fprintf(f, "GridVisible=%d\n", frame->grid_visible ? 1 : 0);
            fprintf(f, "GridScale=%f\n", frame->grid_scale);
            fprintf(f, "Floor=%s\n", CameraFloorToStr(frame->camera_floor));
            fprintf(f, "CubeVisible=%d\n", frame->cube_visible ? 1 : 0);
            fprintf(f, "CubeScale=%f\n", frame->cube_scale);
        }
    }
    fclose(f);
}
