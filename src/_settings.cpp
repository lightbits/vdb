#include <stdio.h>
#include <stdlib.h>

namespace settings
{
    static bool never_ask_on_exit;
    static int window_w;
    static int window_h;
    static int window_x;
    static int window_y;

    void Save(const char *filename)
    {
        FILE *f = fopen(filename, "wb+");
        if (!f)
        {
            printf("Failed to save settings.\n");
            return;
        }
        fprintf(f, "[vdb]\n");
        fprintf(f, "Pos=%d,%d\n", window_x, window_y);
        fprintf(f, "Size=%d,%d\n", window_w, window_h);
        fprintf(f, "NeverAskOnExit=%d\n", never_ask_on_exit);
        fclose(f);
    }

    void LoadOrDefault(const char *filename)
    {
        window_x = -1;
        window_y = -1;
        window_w = 1000;
        window_h = 600;
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
            if      (sscanf(line, "Pos=%d,%d", &x, &y) == 2)     { window_x = x; window_y = y; }
            else if (sscanf(line, "Size=%d,%d", &x, &y) == 2)    { window_w = x; window_h = y; }
            else if (sscanf(line, "NeverAskOnExit=%d", &i) == 1) { never_ask_on_exit = (i != 0); }
        }
        free(line);
        fclose(f);
    }
}
