#include <stdio.h>
#include <stdlib.h>

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
    bool never_ask_on_exit;

    void LoadOrDefault(const char *filename);
    void Save(const char *filename);
};

static settings_t settings;

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

    FILE *f = fopen(filename, "rb");
    if (!f) return;
    if (fseek(f, 0, SEEK_END)) { fclose(f); return; }
    int file_size = (int)ftell(f);
    if (file_size <= 0)        { fclose(f); return; }
    if (fseek(f, 0, SEEK_SET)) { fclose(f); return; }

    char *line = (char*)malloc(file_size);
    while (fgets(line, file_size, f))
    {
        int x,y;
        int i;
        float f;
        if      (sscanf(line, "Pos=%d,%d", &x, &y) == 2)        { window.x = x; window.y = y; }
        else if (sscanf(line, "Size=%d,%d", &x, &y) == 2)       { window.width = x; window.height = y; }
        else if (sscanf(line, "NeverAskOnExit=%d", &i) == 1)    { never_ask_on_exit = (i != 0); }
        else if (sscanf(line, "MouseSensitivity=%f", &f) == 1)  { camera.mouse_sensitivity = f; }
        else if (sscanf(line, "ScrollSensitivity=%f", &f) == 1) { camera.scroll_sensitivity = f; }
        else if (sscanf(line, "MoveSpeedNormal=%f", &f) == 1)   { camera.move_speed_normal = f; }
        else if (sscanf(line, "MoveSpeedSlow=%f", &f) == 1)     { camera.move_speed_slow = f; }
        else if (sscanf(line, "Kp_zoom=%f", &f) == 1)           { camera.Kp_zoom = f; }
        else if (sscanf(line, "Kp_translate=%f", &f) == 1)      { camera.Kp_translate = f; }
        else if (sscanf(line, "Kp_rotate=%f", &f) == 1)         { camera.Kp_rotate = f; }
    }
    free(line);
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
    fprintf(f, "MouseSensitivity=%f\n", camera.mouse_sensitivity);
    fprintf(f, "ScrollSensitivity=%f\n", camera.scroll_sensitivity);
    fprintf(f, "MoveSpeedNormal=%f\n", camera.move_speed_normal);
    fprintf(f, "MoveSpeedSlow=%f\n", camera.move_speed_slow);
    fprintf(f, "Kp_zoom=%f\n", camera.Kp_zoom);
    fprintf(f, "Kp_translate=%f\n", camera.Kp_translate);
    fprintf(f, "Kp_rotate=%f\n", camera.Kp_rotate);
    fclose(f);
}
