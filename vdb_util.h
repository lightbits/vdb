#define vdbViewportN(x, y, w, h) _vdbViewportN(Input.WindowWidth, Input.WindowHeight, x, y, w, h)
void _vdbViewportN(float window_width, float window_height, float x, float y, float w, float h)
{
    glViewport(x * window_width, y * window_height,
               w * window_width, h * window_height);
}

void vdbClear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void vdbDrawCircle(float x, float y, float r, int n = 64)
{
    for (int i = 0; i < n; i++)
    {
        int i1 = i;
        int i2 = i+1;
        float t1 = 6.28318530718f*i1 / (float)n;
        float t2 = 6.28318530718f*i2 / (float)n;
        glVertex2f(x+r*cosf(t1), y+r*sinf(t1));
        glVertex2f(x+r*cosf(t2), y+r*sinf(t2));
    }
}

void vdbFillCircle(float x, float y, float r, int n = 64)
{
    for (int i = 0; i < n; i++)
    {
        int i1 = i;
        int i2 = i+1;
        float t1 = 6.28318530718f*i1 / (float)n;
        float t2 = 6.28318530718f*i2 / (float)n;
        glVertex2f(x, y);
        glVertex2f(x+r*cosf(t1), y+r*sinf(t1));
        glVertex2f(x+r*cosf(t2), y+r*sinf(t2));
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
    static float radius = 1.0f;
    static float htheta = SO_PI/2.0f-0.3f;
    static float vtheta = 0.3f;
    static float Rradius = radius;
    static float Rhtheta = htheta;
    static float Rvtheta = vtheta;

    float dt = Input.DeltaTime;
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
void glTexCoord2f(vec2 texel) { glTexCoord2f(texel.x, texel.y); }

vec2 vdbProjectFisheyePoint(float f, vec3 p, float u0, float v0)
// Output y is 0 at top of screen at height at bottom of screen
{
    float l = sqrtf(p.x*p.x+p.y*p.y);
    float t = atanf(-l/p.z);
    float r = f*t;
    float du, dv;
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
    float u = u0 + du;
    float v = v0 - dv;
    vec2 result = { u, v };
    return result;
}

void vdbDrawLineFisheye(mat3 R, vec3 T, float f, float u0, float v0, vec3 p1, vec3 p2, int n = 64)
// Output y is 0 at top of screen at height at bottom of screen
{
    p1 = R*(p1-T);
    p2 = R*(p2-T);
    for (int i = 0; i < n; i++)
    {
        float t1 = i / (float)n;
        float t2 = (i+1) / (float)n;
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

void vdbDrawPointsFisheye(mat3 R, vec3 T, float f, float u0, float v0, vec3 p1, vec3 p2, int n = 64)
// Output y is 0 at top of screen at height at bottom of screen
{
    p1 = R*(p1-T);
    p2 = R*(p2-T);
    for (int i = 0; i < n; i++)
    {
        float t = i / (float)n;
        vec3 p = p1 + (p2-p1)*t;
        vec2 c = vdbProjectFisheyePoint(f, p, u0, v0);
        glVertex2f(c.x, c.y);
    }
}

void vdbDrawLinePinhole(mat3 R, vec3 T, float f, float u0, float v0, float zn, vec3 p1, vec3 p2)
// p1, p2: In world space
// u0, v0: Image center
// zn, zf: Near and far clip planes
// T: p^w_{c/w}
// R: R^c_w
// Output y is 0 at top of screen at height at bottom of screen
{
    p1 = R*(p1 - T);
    p2 = R*(p2 - T);
    float z1 = p1.z;
    float z2 = p2.z;
    vec3 clip1 = p1;
    vec3 clip2 = p2;
    if (-z1 < zn)
    {
        float t = (-zn - z2) / (z1 - z2);
        clip1 = p2 + (p1 - p2)*t;
    }
    if (-z2 < zn)
    {
        float t = (-zn - z1) / (z2 - z1);
        clip2 = p1 + (p2 - p1)*t;
    }
    float u1 = -f*clip1.x/clip1.z + u0;
    float v1 = +f*clip1.y/clip1.z + v0;
    float u2 = -f*clip2.x/clip2.z + u0;
    float v2 = +f*clip2.y/clip2.z + v0;
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

void vdbOrtho(float left,
              float right,
              float bottom,
              float top,
              // optionally, you can map mouse ndc viewport coordinates
              // into orthographic coordinates
              float mouse_x_ndc = 0.0f,
              float mouse_y_ndc = 0.0f,
              float *mouse_x_ortho = 0,
              float *mouse_y_ortho = 0)
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

    if (mouse_x_ortho && mouse_y_ortho)
    {
        *mouse_x_ortho = left + (right-left)*(0.5f+0.5f*mouse_x_ndc);
        *mouse_y_ortho = bottom + (top-bottom)*(0.5f+0.5f*mouse_y_ndc);
    }
}

void vdbDrawRect(float x, float y, float w, float h)
{
    glVertex2f(x, y);
    glVertex2f(x+w, y);

    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);

    glVertex2f(x+w, y+h);
    glVertex2f(x, y+h);

    glVertex2f(x, y+h);
    glVertex2f(x, y);
}

void vdbFillRect(float x, float y, float w, float h)
{
    glVertex2f(x, y);
    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);
    glVertex2f(x+w, y+h);
    glVertex2f(x, y+h);
    glVertex2f(x, y);
}

void vdbClearFill(float r, float g, float b, float a)
{
    vdbOrtho(-1.0f, +1.0f, -1.0f, +1.0f);
    glBegin(GL_TRIANGLES);
    glColor4f(r, g, b, a);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(+1.0f, -1.0f);
    glVertex2f(+1.0f, +1.0f);
    glVertex2f(+1.0f, +1.0f);
    glVertex2f(-1.0f, +1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
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
    *r = A1 + B1 * sinf(tp * (C1 * t + D1));
    *g = A2 + B2 * sinf(tp * (C2 * t + D2));
    *b = A3 + B3 * sinf(tp * (C3 * t + D3));
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
    float r = A1 + B1 * sinf(tp * (C1 * t + D1));
    float g = A2 + B2 * sinf(tp * (C2 * t + D2));
    float b = A3 + B3 * sinf(tp * (C3 * t + D3));
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

unsigned char *vdbLoadImage(const char *filename, int *width, int *height, int force_channels)
{
    int channels;
    unsigned char *result = stbi_load(filename, width, height, &channels, force_channels);
    SDL_assert(result);
    return result;
}

unsigned char *vdbRGBToGray(unsigned char *in, int w, int h)
{
    unsigned char *out = (unsigned char*)calloc(w*h,1);
    unsigned char *pixel = in;
    for (int i = 0; i < w*h; i++)
    {
        float r = (float)pixel[0];
        float g = (float)pixel[1];
        float b = (float)pixel[2];
        float result_real = (r + r + b + g + g + g) / 6.0f;
        int result_rounded = (int)result_real;
        if (result_rounded < 0) result_rounded = 0;
        if (result_rounded > 255) result_rounded = 255;
        unsigned char result = (unsigned char)result_rounded;

        out[i] = result;
        pixel += 3;
    }
    return out;
}

unsigned char *vdbResizeImage(unsigned char *in, int w, int h, int c, int rw, int rh)
{
    unsigned char *out = (unsigned char*)malloc(rw*rh*c);
    float dx = 1.0f / (float)rw;
    float dy = 1.0f / (float)rh;
    for (int y = 0; y < rh; y++)
    for (int x = 0; x < rw; x++)
    {
        int src_x = (int)(x*dx*w);
        int src_y = (int)(y*dy*h);

        unsigned char *src = &in[(src_y*w+src_x)*c]; // @ bounds
        unsigned char *dst = &out[(y*rw+x)*c];

        for (int i = 0; i < c; i++)
            dst[i] = src[i];
    }
    return out;
}

GLuint vdbLoadTexture(const char *filename,
                      GLenum mag_filter = GL_LINEAR,
                      GLenum min_filter = GL_LINEAR,
                      GLenum wrap_s = GL_CLAMP_TO_EDGE,
                      GLenum wrap_t = GL_CLAMP_TO_EDGE,
                      GLenum internal_format = GL_RGBA)
{
    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
    SDL_assert(data);

    GLuint result = vdbMakeTexture(data, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    min_filter, mag_filter,
                    wrap_s, wrap_t,
                    internal_format);

    stbi_image_free(data);
    return result;
}

void vdbDrawTexture(GLuint texture, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_TRIANGLES);
    glColor4f(r, g, b, a);
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

bool _vdbTakeScreenshot(vdb_input input, char *filename,
                        bool gui, bool cursor)
{
    static bool taking = false;
    if (!taking)
    {
        taking = true;
        static char buf_filename[1024];
        strcpy(buf_filename, filename); // this is dumb

        *input.ScreenshotFilename = buf_filename;
        *input.ScreenshotDrawGui = gui;
        *input.ScreenshotDrawCursor = cursor;
        *input.TakeScreenshotNoDialog = true;
        return false;
    }
    else
    {
        taking = false;
        return true;
    }
}

#define vdbTakeScreenshot(filename, gui, cursor) _vdbTakeScreenshot(Input, filename, gui, cursor)
