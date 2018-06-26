#include <stdio.h>
#include <stdlib.h>
struct vdb_settings_t
{
    bool never_ask_on_exit;
    int window_w;
    int window_h;
    int window_x;
    int window_y;
};

void vdbSaveSettings(vdb_settings_t s, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f)
    {
        printf("Warning: Failed to save settings.\n");
        return;
    }
    fprintf(f, "Pos=%d,%d\n", s.window_x, s.window_y);
    fprintf(f, "Size=%d,%d\n", s.window_w, s.window_h);
    fprintf(f, "NeverAskOnExit=%d\n", s.never_ask_on_exit);
    fclose(f);
}

vdb_settings_t vdbLoadSettingsOrDefault(const char *filename)
{
    vdb_settings_t settings = {0};
    settings.window_x = -1;
    settings.window_y = -1;
    settings.window_w = 1000;
    settings.window_h = 600;
    settings.never_ask_on_exit = false;

    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        return settings;
    }
    int file_size;
    if (fseek(f, 0, SEEK_END) || (file_size = (int)ftell(f)) == -1 || fseek(f, 0, SEEK_SET))
    {
        fclose(f);
        return settings;
    }
    char *line = (char*)malloc(file_size);

    while (fscanf(f, "%s", line) == 1) // todo: more robust line-by-line reading?
    {
        // very rudimentary parser
        int x,y;
        int i;
        if (sscanf(line, "Pos=%d,%d", &x, &y) == 2)          { settings.window_x = x; settings.window_y = y; }
        else if (sscanf(line, "Size=%d,%d", &x, &y) == 2)    { settings.window_w = x; settings.window_h = y; }
        else if (sscanf(line, "NeverAskOnExit=%d", &i) == 1) { settings.never_ask_on_exit = (i != 0); }
    }

    free(line);
    return settings;
}
