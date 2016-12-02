#include "vdb2.h"

#include "lib/so_batch.h"
static float vdb__globals_color_r = 1.0f;
static float vdb__globals_color_g = 1.0f;
static float vdb__globals_color_b = 1.0f;
static float vdb__globals_color_a = 1.0f;
void vdbColor4f(float r, float g, float b, float a)
{
    vdb__globals_color_r = r;
    vdb__globals_color_g = g;
    vdb__globals_color_b = b;
    vdb__globals_color_a = a;
}

void vdbTriangle2f(float x1, float y1, float x2, float y2, float x3, float y3)
{
    float x[] = { x1, x2, x3 };
    float y[] = { y1, y2, y3 };
    sob_vertex v[3] = {0};
    for (int i = 0; i < 3; i++)
    {
        float x_ndc, y_ndc;
        vdbModelToNDC(x[i], y[i], 0.0f, 1.0f, &x_ndc, &y_ndc);
        v[i].x = x_ndc;
        v[i].y = y_ndc;
        v[i].z = 0.0f;
        v[i].w = 1.0f;
        v[i].r = vdb__globals_color_r;
        v[i].g = vdb__globals_color_g;
        v[i].b = vdb__globals_color_b;
        v[i].a = vdb__globals_color_a;
    }
    sob_triangle(v[0], v[1], v[2]);
}

void vdbLine2f(float x1, float y1, float x2, float y2, float width=2.0f)
{
    float x[] = { x1, x2 };
    float y[] = { y1, y2 };
    sob_vertex v[2] = {0};
    for (int i = 0; i < 2; i++)
    {
        float x_ndc, y_ndc;
        vdbModelToNDC(x[i], y[i], 0.0f, 1.0f, &x_ndc, &y_ndc);
        v[i].x = x_ndc;
        v[i].y = y_ndc;
        v[i].z = 0.0f;
        v[i].w = 1.0f;
        v[i].r = vdb__globals_color_r;
        v[i].g = vdb__globals_color_g;
        v[i].b = vdb__globals_color_b;
        v[i].a = vdb__globals_color_a;
    }
    sob_line(v[0], v[1], width);
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

void vdbFillCircle2f(float x, float y, float r, int n=16)
{
    for (int ti = 0; ti < n; ti++)
    {
        float t1 = 2.0f*3.1415926f*(ti+0)/n;
        float t2 = 2.0f*3.1415926f*(ti+1)/n;
        vdbTriangle2f(x, y,
                      x+r*cosf(t1), y+r*sinf(t1),
                      x+r*cosf(t2), y+r*sinf(t2));
    }
}

int main(int, char **)
{
    VDBB("");
    {
        // Text("%d", vdb_countof(vdb_builtin_palette));

        // vdbClear(vdbPalette(1));

        // vdbSphereCamera(0, 0, 0);
        // vdbSphereCameraOrtho(0, 0, 0);

        // vdbFreeSphereCamera();

        // glLines(4.0f);
        // glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        // vdbGridXY(-1.0f, +1.0f, -1.0f, +1.0f, 16);
        // glEnd();

        // vdbOrtho(0.0f, 4.0f, 0.0f, 4.0f);
        // for (int y = 0; y < 4; y++)
        // for (int x = 0; x < 4; x++)
        // {
        //     float u = x+0.5f;
        //     float v = y+0.5f;
        //     vdbColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        //     vdbPoint2f(u, v, 8.0f);

        //     if (vdbMap(u, v))
        //     {
        //         vdbColor4f(1.0f, 1.0f, 0.1f, 1.0f);
        //         vdbPoint2f(u, v, 16.0f);

        //         vdbColor4f(0.2f, 0.4f, 1.0f, 1.0f);
        //         vdbLine2f(u, v, u+0.2f, v);
        //         vdbLine2f(u, v, u, v+0.2f);
        //         vdbLine2f(u, v, u-0.2f, v);
        //         vdbLine2f(u, v, u, v-0.2f);

        //         SetTooltip("%d %d", x, y);
        //     }
        // }

        // sob_draw();

        vdbClear(1.0f, 1.0f, 1.0f, 1.0f);

        vdbSquareViewport();
        vdbOrtho(-1.2f, +1.2f, -1.2f, +1.2f);

        // glLines(2.0f);
        // glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
        // vdbGridXY(-1.0f, +1.0f, -1.0f, +1.0f, 32);
        // glEnd();

        static int N_CIRCLES = 3;
        static float ARCLEN = 0.1f;
        vdb_for(k, 0, N_CIRCLES)
        {
            float r = (k+1.0f)/N_CIRCLES;
            int steps = (int)(1 + SO_TWO_PI*r/ARCLEN);
            float delta = ARCLEN/r;
            float t = delta;
            while (t < SO_TWO_PI)
            {
                float x = r*cosf(t);
                float y = r*sinf(t);
                vdbColor4f(0.1f, 0.5f, 1.0f, 1.0f);
                vdbPoint2f(x, y, 10.0f);
                t += delta;
            }
        }

        SliderFloat("ARCLEN", &ARCLEN, 0.01f, 1.0f);
        SliderInt("N_CIRCLES", &N_CIRCLES, 0, 32);

        sob_draw();
    }
    VDBE();

    return 0;
}
