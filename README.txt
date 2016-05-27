Graphical Debugger
------------------

Sort of like a programmable Simulink scope that you can insert into your C++ code to visualize your data.

Usage
-----
See test.cpp

Todo
----
* Store skipped windows in imgui.ini
* Return hover position and belonging item
    In callback function?
* Better sliders. When moving mouse slowly: Higher precision, fast: large steps?

Done
----
* Allow to set (and remember) window size and position
* Screenshot and note taking
* Quick exit

Todo: Zoom
----------
#if 0
static r32 width = 2.0f;
static r32 height = 2.0f;
static r32 center_x = 0.0f;
static r32 center_y = 0.0f;
r32 left = center_x-width/2.0f;
r32 right = center_x+width/2.0f;
r32 bottom = center_y-height/2.0f;
r32 top = center_y+height/2.0f;
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
float Ax = 2.0f / (right-left);
float Bx = -1.0f - Ax*left;
float Ay = 2.0f / (top-bottom);
float By = -1.0f - Ay*bottom;
float projection[] = {
    Ax,   0.0f, 0.0f, 0.0f,
    0.0f, Ay,   0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    Bx,   By,   0.0f, 1.0f
};
glLoadMatrixf(projection);

static int zoom_state = 0;
static r32 zoom_x0 = 0.0f;
static r32 zoom_y0 = 0.0f;

if (zoom_state == 2)
{
    r32 zoom_x1 = left + (right-left)*(0.5f+0.5f*MOUSEX);
    r32 zoom_y1 = bottom + (top-bottom)*(0.5f-0.5f*MOUSEY);
    vdbOrtho(-1.0f, +1.0f, -1.0f, +1.0f);
    glBegin(GL_LINES);
    glColor4f(1.0f, 0.2f, 0.1f, 1.0f);
    glVertex2f(zoom_x0, zoom_y0);
    glVertex2f(zoom_x1, zoom_y0);

    glVertex2f(zoom_x0, zoom_y0);
    glVertex2f(zoom_x0, zoom_y1);

    glVertex2f(zoom_x0, zoom_y1);
    glVertex2f(zoom_x1, zoom_y1);

    glVertex2f(zoom_x1, zoom_y0);
    glVertex2f(zoom_x1, zoom_y1);
    glEnd();

    if (Input.Mouse.Left.Released)
    {
        width = abs(zoom_x1-zoom_x0);
        height = abs(zoom_y1-zoom_y0);
        center_x = (zoom_x0+zoom_x1)/2.0f;
        center_y = (zoom_y0+zoom_y1)/2.0f;
        zoom_state = 0;
    }
}

if (zoom_state == 1)
{
    if (Input.Mouse.Left.Released)
    {
        zoom_x0 = left + (right-left)*(0.5f+0.5f*MOUSEX);
        zoom_y0 = bottom + (top-bottom)*(0.5f-0.5f*MOUSEY);
        zoom_state = 2;
    }
}

if (Button("Zoom"))
{
    zoom_state = 1;
}

if (Button("Reset zoom"))
{
    width = 2.0f;
    height = 2.0f,
    center_x = 0.0f;
    center_y = 0.0f;
}
#endif
