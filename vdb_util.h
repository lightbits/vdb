#define COLOR_WHITE  0.75f, 0.7f, 0.65f, 1.0f
#define COLOR_BLACK  0.0f, 0.0f, 0.0f, 1.0f
#define COLOR_UNITY  0.15f, 0.13f, 0.1f, 1.0f
#define COLOR_RED    1.0f, 0.2f, 0.1f, 1.0f
#define COLOR_GREEN  0.1f, 1.0f, 0.2f, 1.0f
#define COLOR_BLUE   0.15f, 0.35f, 0.85f, 1.0f
#define COLOR_YELLOW 1.0f, 1.0f, 0.2f, 1.0f
#define COLOR_CREAMY 0.8f, 0.7f, 0.51f, 1.0f

void vdbClear(r32 r, r32 g, r32 b, r32 a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void vdbFillCircle(r32 x, r32 y, r32 r, int n = 64)
{
    for (int i = 0; i < n; i++)
    {
        int i1 = i;
        int i2 = i+1;
        r32 t1 = SO_TWO_PI*i1 / (r32)n;
        r32 t2 = SO_TWO_PI*i2 / (r32)n;
        glVertex2f(x, y);
        glVertex2f(x+r*cos(t1), y+r*sin(t1));
        glVertex2f(x+r*cos(t2), y+r*sin(t2));
    }
}

#ifndef VDB_NO_MATH
void vdbView3D(mat4 model, mat4 view, mat4 projection)
{
    mat4 pvm = projection * view * model;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(pvm.data);
}

mat4 vdbCamera3D(vdb_input Input, vec3 focus = m_vec3(0.0f, 0.0f, 0.0f))
{
    static r32 radius = 1.0f;
    static r32 htheta = SO_PI/2.0f-0.3f;
    static r32 vtheta = 0.3f;
    static r32 Rradius = radius;
    static r32 Rhtheta = htheta;
    static r32 Rvtheta = vtheta;

    r32 dt = Input.DeltaTime;
    if (vdbKeyDown(LSHIFT))
    {
        if (vdbKeyPressed(Z))
            Rradius /= 2.0f;
        if (vdbKeyPressed(X))
            Rradius *= 2.0f;
        if (vdbKeyPressed(LEFT))
            Rhtheta -= SO_PI / 4.0f;
        if (vdbKeyPressed(RIGHT))
            Rhtheta += SO_PI / 4.0f;
        if (vdbKeyPressed(UP))
            Rvtheta -= SO_PI / 4.0f;
        if (vdbKeyPressed(DOWN))
            Rvtheta += SO_PI / 4.0f;
    }
    else
    {
        if (vdbKeyDown(Z))
            Rradius -= dt;
        if (vdbKeyDown(X))
            Rradius += dt;
        if (vdbKeyDown(LEFT))
            Rhtheta -= dt;
        if (vdbKeyDown(RIGHT))
            Rhtheta += dt;
        if (vdbKeyDown(UP))
            Rvtheta -= dt;
        if (vdbKeyDown(DOWN))
            Rvtheta += dt;
    }

    radius += 10.0f * (Rradius - radius) * dt;
    htheta += 10.0f * (Rhtheta - htheta) * dt;
    vtheta += 10.0f * (Rvtheta - vtheta) * dt;

    mat3 R = m_mat3(mat_rotate_z(htheta)*mat_rotate_x(vtheta));
    vec3 p = focus + R.a3 * radius;
    mat4 c_to_w = m_se3(R, p);
    return m_se3_inverse(c_to_w);
}

void glVertex2f(vec2 p) { glVertex2f(p.x, p.y); }
void glLine2f(vec2 a, vec2 b) { glVertex2f(a); glVertex2f(b); }
void glVertex3f(vec3 p) { glVertex3f(p.x, p.y, p.z); }
void glLine3f(vec3 a, vec3 b) { glVertex3f(a); glVertex3f(b); }

vec2 vdbProjectFisheyePoint(r32 f, vec3 p, r32 u0, r32 v0)
// Output y is 0 at top of screen at height at bottom of screen
{
    r32 l = sqrt(p.x*p.x+p.y*p.y);
    r32 t = atan(-l/p.z);
    r32 r = f*t;
    r32 du, dv;
    if (t < 0.001f)
    {
        du = 0.0f;
        dv = 0.0f;
    }
    else
    {
        du = r*p.x/l;
        dv = r*p.y/l;
    }
    r32 u = u0 + du;
    r32 v = v0 - dv;
    vec2 result = { u, v };
    return result;
}

void vdbDrawLineFisheye(mat3 R, vec3 T, r32 f, r32 u0, r32 v0, vec3 p1, vec3 p2, s32 n = 64)
// Output y is 0 at top of screen at height at bottom of screen
{
    p1 = R*(p1-T);
    p2 = R*(p2-T);
    for (s32 i = 0; i < n; i++)
    {
        r32 t1 = i / (r32)n;
        r32 t2 = (i+1) / (r32)n;
        vec3 l1 = p1 + (p2-p1)*t1;
        vec3 l2 = p1 + (p2-p1)*t2;
        if (l1.z > 0.0f || l2.z > 0.0f)
            continue;
        vec2 l1c = vdbProjectFisheyePoint(f, l1, u0, v0);
        vec2 l2c = vdbProjectFisheyePoint(f, l2, u0, v0);
        glVertex2f(l1c.x, l1c.y);
        glVertex2f(l2c.x, l2c.y);
    }
}

void vdbDrawPointsFisheye(mat3 R, vec3 T, r32 f, r32 u0, r32 v0, vec3 p1, vec3 p2, s32 n = 64)
// Output y is 0 at top of screen at height at bottom of screen
{
    p1 = R*(p1-T);
    p2 = R*(p2-T);
    for (s32 i = 0; i < n; i++)
    {
        r32 t = i / (r32)n;
        vec3 p = p1 + (p2-p1)*t;
        vec2 c = vdbProjectFisheyePoint(f, p, u0, v0);
        glVertex2f(c.x, c.y);
    }
}

void vdbDrawLinePinhole(mat3 R, vec3 T, r32 f, r32 u0, r32 v0, r32 zn, vec3 p1, vec3 p2)
// p1, p2: In world space
// u0, v0: Image center
// zn, zf: Near and far clip planes
// T: p^w_{c/w}
// R: R^c_w
// Output y is 0 at top of screen at height at bottom of screen
{
    p1 = R*(p1 - T);
    p2 = R*(p2 - T);
    r32 z1 = p1.z;
    r32 z2 = p2.z;
    vec3 clip1 = p1;
    vec3 clip2 = p2;
    if (-z1 < zn)
    {
        r32 t = (-zn - z2) / (z1 - z2);
        clip1 = p2 + (p1 - p2)*t;
    }
    if (-z2 < zn)
    {
        r32 t = (-zn - z1) / (z2 - z1);
        clip2 = p1 + (p2 - p1)*t;
    }
    r32 u1 = -f*clip1.x/clip1.z + u0;
    r32 v1 = +f*clip1.y/clip1.z + v0;
    r32 u2 = -f*clip2.x/clip2.z + u0;
    r32 v2 = +f*clip2.y/clip2.z + v0;
    glVertex2f(u1, v1);
    glVertex2f(u2, v2);
}
#endif

void vdbGridXY(r32 x_min, r32 x_max, r32 y_min, r32 y_max, int steps)
{
    for (int i = 0; i <= steps; i++)
    {
        glVertex3f(x_min, y_min + (y_max-y_min)*i/steps, 0.0f);
        glVertex3f(x_max, y_min + (y_max-y_min)*i/steps, 0.0f);

        glVertex3f(x_min + (x_max-x_min)*i/steps, y_min, 0.0f);
        glVertex3f(x_min + (x_max-x_min)*i/steps, y_max, 0.0f);
    }
}

void glPoints(r32 size) { glPointSize(size); glBegin(GL_POINTS); }
void glLines(r32 width) { glLineWidth(width); glBegin(GL_LINES); }

void vdbOrtho(r32 left, r32 right, r32 bottom, r32 top)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    r32 Ax = 2.0f / (right-left);
    r32 Bx = -1.0f - Ax*left;
    r32 Ay = 2.0f / (top-bottom);
    r32 By = -1.0f - Ay*bottom;
    r32 modelview[] = {
        Ax,   0.0f, 0.0f, 0.0f,
        0.0f, Ay,   0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        Bx,   By,   0.0f, 1.0f
    };
    glLoadMatrixf(modelview);
}

void vdbFlippedY(bool enabled)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (enabled)
    {
        r32 projection[] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glLoadMatrixf(projection);
    }
}

void vdbAdditiveBlend()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
}

void vdbAlphaBlend()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void vdbNoBlend()
{
    glDisable(GL_BLEND);
}

void vdbColorRamp(r32 t, r32 *r, r32 *g, r32 *b)
{
    r32 A1 = 0.54f;
    r32 A2 = 0.55f;
    r32 A3 = 0.56f;
    r32 B1 = 0.5f;
    r32 B2 = 0.5f;
    r32 B3 = 0.7f;
    r32 C1 = 0.5f;
    r32 C2 = 0.5f;
    r32 C3 = 0.5f;
    r32 D1 = 0.7f;
    r32 D2 = 0.8f;
    r32 D3 = 0.88f;
    r32 tp = 3.1415926f*2.0f;
    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;
    *r = A1 + B1 * sin(tp * (C1 * t + D1));
    *g = A2 + B2 * sin(tp * (C2 * t + D2));
    *b = A3 + B3 * sin(tp * (C3 * t + D3));
}

void vdbColorRamp(r32 t)
{
    r32 A1 = 0.54f;
    r32 A2 = 0.55f;
    r32 A3 = 0.56f;
    r32 B1 = 0.5f;
    r32 B2 = 0.5f;
    r32 B3 = 0.7f;
    r32 C1 = 0.5f;
    r32 C2 = 0.5f;
    r32 C3 = 0.5f;
    r32 D1 = 0.7f;
    r32 D2 = 0.8f;
    r32 D3 = 0.88f;
    r32 tp = 3.1415926f*2.0f;
    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;
    r32 r = A1 + B1 * sin(tp * (C1 * t + D1));
    r32 g = A2 + B2 * sin(tp * (C2 * t + D2));
    r32 b = A3 + B3 * sin(tp * (C3 * t + D3));
    glColor4f(r, g, b, 1.0f);
}

GLuint vdbMakeTexture(void *data, int width, int height,
                      GLenum data_format,
                      GLenum data_type = GL_UNSIGNED_BYTE,
                      GLenum mag_filter = GL_LINEAR,
                      GLenum min_filter = GL_LINEAR,
                      GLenum wrap_s = GL_CLAMP_TO_EDGE,
                      GLenum wrap_t = GL_CLAMP_TO_EDGE,
                      GLenum internal_format = GL_RGBA)
{
    GLuint result = 0;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 internal_format,
                 width,
                 height,
                 0,
                 data_format,
                 data_type,
                 data);
    // glGenerateMipmap(GL_TEXTURE_2D); // todo
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glBindTexture(GL_TEXTURE_2D, 0);
    return result;
}

u08 *vdbLoadImage(char *filename, int *width, int *height, int force_channels)
{
    int channels;
    u08 *result = stbi_load(filename, width, height, &channels, force_channels);
    SDL_assert(result);
    return result;
}

u08 *vdbRGBToGray(u08 *in, int w, int h)
{
    u08 *out = (u08*)calloc(w*h,1);
    u08 *pixel = in;
    for (s32 i = 0; i < w*h; i++)
    {
        r32 r = (r32)pixel[0];
        r32 g = (r32)pixel[1];
        r32 b = (r32)pixel[2];
        r32 result_real = (r + r + b + g + g + g) / 6.0f;
        s32 result_rounded = (s32)result_real;
        if (result_rounded < 0) result_rounded = 0;
        if (result_rounded > 255) result_rounded = 255;
        u08 result = (u08)result_rounded;

        out[i] = result;
        pixel += 3;
    }
    return out;
}

u08 *vdbResizeImage(u08 *in, int w, int h, int c, int rw, int rh)
{
    u08 *out = (u08*)malloc(rw*rh*c);
    r32 dx = 1.0f / (r32)rw;
    r32 dy = 1.0f / (r32)rh;
    for (int y = 0; y < rh; y++)
    for (int x = 0; x < rw; x++)
    {
        int src_x = (int)(x*dx*w);
        int src_y = (int)(y*dy*h);

        u08 *src = &in[(src_y*w+src_x)*c]; // @ bounds
        u08 *dst = &out[(y*rw+x)*c];

        for (int i = 0; i < c; i++)
            dst[i] = src[i];
    }
    return out;
}

GLuint vdbLoadTexture(char *filename,
                      GLenum mag_filter = GL_LINEAR,
                      GLenum min_filter = GL_LINEAR,
                      GLenum wrap_s = GL_CLAMP_TO_EDGE,
                      GLenum wrap_t = GL_CLAMP_TO_EDGE,
                      GLenum internal_format = GL_RGBA)
{
    int width, height, channels;
    u08 *data = stbi_load(filename, &width, &height, &channels, 4);
    SDL_assert(data);

    GLuint result = vdbMakeTexture(data, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    min_filter, mag_filter,
                    wrap_s, wrap_t,
                    internal_format);

    stbi_image_free(data);
    return result;
}

void vdbDrawTexture(GLuint texture)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(+1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(+1.0f, +1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(+1.0f, +1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, +1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void vdbImage(void *data, int width, int height,
              GLenum data_format,
              GLenum data_type = GL_UNSIGNED_BYTE,
              GLenum mag_filter = GL_LINEAR,
              GLenum min_filter = GL_LINEAR,
              GLenum wrap_s = GL_CLAMP_TO_EDGE,
              GLenum wrap_t = GL_CLAMP_TO_EDGE,
              GLenum internal_format = GL_RGBA)
{
    static GLuint texture = 0;
    if (vdb_isFirstLoopIteration())
    {
        glDeleteTextures(1, &texture);
        texture = vdbMakeTexture(data, width, height,
                                 data_format, data_type,
                                 mag_filter, min_filter,
                                 wrap_s, wrap_t,
                                 internal_format);
    }
    vdbDrawTexture(texture);
}

void _vdbTakeScreenshot(vdb_input input, char *filename,
                        bool gui, bool cursor)
{
    static char buf_filename[1024];
    strcpy(buf_filename, filename); // this is dumb

    *input.ScreenshotFilename = buf_filename;
    *input.ScreenshotDrawGui = gui;
    *input.ScreenshotDrawCursor = cursor;
    *input.TakeScreenshotNoDialog = true;
}

#define vdbTakeScreenshot(filename, gui, cursor) _vdbTakeScreenshot(Input, filename, gui, cursor)
