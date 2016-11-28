#include "vdb2.h"
#include "lib/so_batch.h"

float vdb__globals_color_r;
float vdb__globals_color_g;
float vdb__globals_color_b;
float vdb__globals_color_a;
void vdbColor4f(float r, float g, float b, float a)
{
    vdb__globals_color_r = r;
    vdb__globals_color_g = g;
    vdb__globals_color_b = b;
    vdb__globals_color_a = a;
}

void vdbPoint2f(float x, float y, float size=4.0f)
{
    float x_ndc, y_ndc;
    vdbModelToNDC(x, y, 0.0f, 1.0f, &x_ndc, &y_ndc);
    {
        sob_vertex v = {0};
        v.x = x_ndc;
        v.y = y_ndc;
        v.z = 0.0f;
        v.w = 1.0f;
        v.r = vdb__globals_color_r;
        v.g = vdb__globals_color_g;
        v.b = vdb__globals_color_b;
        v.a = vdb__globals_color_a;
        sob_point(v, size);
    }
}

void vdbLine2f(float x1, float y1, float x2, float y2, float width=2.0f)
{
    float x1_ndc, y1_ndc, x2_ndc, y2_ndc;
    vdbModelToNDC(x1, y1, 0.0f, 1.0f, &x1_ndc, &y1_ndc);
    vdbModelToNDC(x2, y2, 0.0f, 1.0f, &x2_ndc, &y2_ndc);
    {
        sob_vertex v1 = {0};
        v1.x = x1_ndc;
        v1.y = y1_ndc;
        v1.z = 0.0f;
        v1.w = 1.0f;
        v1.r = vdb__globals_color_r;
        v1.g = vdb__globals_color_g;
        v1.b = vdb__globals_color_b;
        v1.a = vdb__globals_color_a;

        sob_vertex v2 = {0};
        v2.x = x2_ndc;
        v2.y = y2_ndc;
        v2.z = 0.0f;
        v2.w = 1.0f;
        v2.r = vdb__globals_color_r;
        v2.g = vdb__globals_color_g;
        v2.b = vdb__globals_color_b;
        v2.a = vdb__globals_color_a;

        sob_line(v1, v2, width);
    }
}

int main(int, char **)
{
    VDBB("");
    {
        vdbClear(vdbPalette4i(0));

        vdbOrtho(0.0f, 4.0f, 0.0f, 4.0f);
        for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
        {
            float u = x+0.5f;
            float v = y+0.5f;
            vdbColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            vdbPoint2f(u, v, 8.0f);

            if (vdbMap(u, v))
            {
                vdbColor4f(1.0f, 1.0f, 0.1f, 1.0f);
                vdbPoint2f(u, v, 16.0f);

                vdbColor4f(0.2f, 0.4f, 1.0f, 1.0f);
                vdbLine2f(u, v, u+0.2f, v);
                vdbLine2f(u, v, u, v+0.2f);
                vdbLine2f(u, v, u-0.2f, v);
                vdbLine2f(u, v, u, v-0.2f);

                SetTooltip("%d %d", x, y);
            }
        }

        sob_draw();
    }
    VDBE();

    return 0;
}
