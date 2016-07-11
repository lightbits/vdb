#define COLOR_WHITE  0.75f, 0.7f, 0.65f, 1.0f
#define COLOR_BLACK  0.0f, 0.0f, 0.0f, 1.0f
#define COLOR_UNITY  0.15f, 0.13f, 0.1f, 1.0f
#define COLOR_RED    1.0f, 0.2f, 0.1f, 1.0f
#define COLOR_GREEN  0.1f, 1.0f, 0.2f, 1.0f
#define COLOR_BLUE   0.1f, 0.2f, 1.0f, 1.0f
#define COLOR_YELLOW 1.0f, 1.0f, 0.2f, 1.0f
#define COLOR_CREAMY 0.8f, 0.7f, 0.51f, 1.0f

void vdbClear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

#ifndef VDB_NO_MATH
void vdbView3D(mat4 model, mat4 view, mat4 projection)
{
    mat4 pvm = projection * view * model;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(pvm.data);
}

void glVertex2f(vec2 p) { glVertex2f(p.x, p.y); }
void glLine2f(vec2 a, vec2 b) { glVertex2f(a); glVertex2f(b); }
void glVertex3f(vec3 p) { glVertex3f(p.x, p.y, p.z); }
void glLine3f(vec3 a, vec3 b) { glVertex3f(a); glVertex3f(b); }

vec2 vdbProjectFisheyePoint(r32 f, vec3 p)
{
    p = m_normalize(p);
    r32 theta = acos(-p.z);
    r32 r = f*theta;
    r32 l = sqrt(p.x*p.x+p.y*p.y);
    r32 x, y;
    if (l < 0.001f)
    {
        x = 0.0f;
        y = 0.0f;
    }
    else
    {
        x = r*p.x/l;
        y = r*p.y/l;
    }
    return m_vec2(x, y);
}

void vdbDrawLineFisheye(mat3 R, vec3 T, r32 f, r32 u0, r32 v0, vec3 p1, vec3 p2, s32 n = 64)
{
    p1 = R*(p1-T);
    p2 = R*(p2-T);
    for (s32 i = 0; i < n; i++)
    {
        r32 t1 = i / (r32)n;
        r32 t2 = (i+1) / (r32)n;
        vec3 l1 = p1 + (p2-p1)*t1;
        vec3 l2 = p1 + (p2-p1)*t2;
        vec2 l1c = vdbProjectFisheyePoint(f, l1);
        vec2 l2c = vdbProjectFisheyePoint(f, l2);
        glVertex2f(l1c.x+u0, l1c.y+v0);
        glVertex2f(l2c.x+u0, l2c.y+v0);
    }
}

void vdbDrawLinePinhole(mat3 R, vec3 T, r32 f, r32 u0, r32 v0, r32 zn, vec3 p1, vec3 p2)
// p1, p2: In world space
// u0, v0: Image center
// zn, zf: Near and far clip planes
// T: p^w_{c/w}
// R: R^c_w
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
    r32 v1 = -f*clip1.y/clip1.z + v0;
    r32 u2 = -f*clip2.x/clip2.z + u0;
    r32 v2 = -f*clip2.y/clip2.z + v0;
    glVertex2f(u1, v1);
    glVertex2f(u2, v2);
}
#endif

void vdbGridXY(float x_min, float x_max, float y_min, float y_max, int steps)
{
    for (int i = 0; i <= steps; i++)
    {
        glVertex3f(x_min, y_min + (y_max-y_min)*i/steps, 0.0f);
        glVertex3f(x_max, y_min + (y_max-y_min)*i/steps, 0.0f);

        glVertex3f(x_min + (x_max-x_min)*i/steps, y_min, 0.0f);
        glVertex3f(x_min + (x_max-x_min)*i/steps, y_max, 0.0f);
    }
}

void glPoints(float size) { glPointSize(size); glBegin(GL_POINTS); }
void glLines(float width) { glLineWidth(width); glBegin(GL_LINES); }

void vdbOrtho(float left, float right, float bottom, float top)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float Ax = 2.0f / (right-left);
    float Bx = -1.0f - Ax*left;
    float Ay = 2.0f / (top-bottom);
    float By = -1.0f - Ay*bottom;
    float modelview[] = {
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
        float projection[] = {
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

void vdbColorRamp(float t, float *r, float *g, float *b)
{
    float A1 = 0.54f;
    float A2 = 0.55f;
    float A3 = 0.56f;
    float B1 = 0.5f;
    float B2 = 0.5f;
    float B3 = 0.7f;
    float C1 = 0.5f;
    float C2 = 0.5f;
    float C3 = 0.5f;
    float D1 = 0.7f;
    float D2 = 0.8f;
    float D3 = 0.88f;
    float tp = 3.1415926f*2.0f;
    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;
    *r = A1 + B1 * sin(tp * (C1 * t + D1));
    *g = A2 + B2 * sin(tp * (C2 * t + D2));
    *b = A3 + B3 * sin(tp * (C3 * t + D3));
}

void vdbColorRamp(float t)
{
    float A1 = 0.54f;
    float A2 = 0.55f;
    float A3 = 0.56f;
    float B1 = 0.5f;
    float B2 = 0.5f;
    float B3 = 0.7f;
    float C1 = 0.5f;
    float C2 = 0.5f;
    float C3 = 0.5f;
    float D1 = 0.7f;
    float D2 = 0.8f;
    float D3 = 0.88f;
    float tp = 3.1415926f*2.0f;
    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;
    float r = A1 + B1 * sin(tp * (C1 * t + D1));
    float g = A2 + B2 * sin(tp * (C2 * t + D2));
    float b = A3 + B3 * sin(tp * (C3 * t + D3));
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
