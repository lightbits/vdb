#include "_matrix.cpp"

static vdbMat4 vdb_projection = {0};
static vdbMat4 vdb_modelview = {0};
static vdbMat4 vdb_pvm = {0};

void vdbProjection(float *m)
{
    glMatrixMode(GL_PROJECTION);
    if (m)
    {
        vdb_projection = *(vdbMat4*)m;
        #ifdef VDB_MATRIX_ROW_MAJOR
        glLoadMatrixf(m);
        #else
        glLoadTransposeMatrixf(m);
        #endif
    }
    else
    {
        vdb_projection = vdbMatIdentity();
        glLoadIdentity();
    }
    vdb_pvm = vdbMul4x4(vdb_projection, vdb_modelview);
}

void vdbMatrix(float *m)
{
    glMatrixMode(GL_MODELVIEW);
    if (m)
    {
        vdb_modelview = *(vdbMat4*)m;
        #ifdef VDB_MATRIX_ROW_MAJOR
        glLoadMatrixf(m);
        #else
        glLoadTransposeMatrixf(m);
        #endif
    }
    else
    {
        vdb_modelview = vdbMatIdentity();
        glLoadIdentity();
    }
    vdb_pvm = vdbMul4x4(vdb_projection, vdb_modelview);
}

void vdbViewport(float left, float bottom, float width, float height)
{
    vdb.viewport_left = (int)(left*vdb.framebuffer_width);
    vdb.viewport_width = (int)(width*vdb.framebuffer_width);
    vdb.viewport_bottom = (int)(bottom*vdb.framebuffer_height);
    vdb.viewport_height = (int)(height*vdb.framebuffer_height);
    glViewport(vdb.viewport_left, vdb.viewport_bottom,
               vdb.viewport_width, vdb.viewport_height);
}

void vdbOrtho(float x_left, float x_right, float y_bottom, float y_top)
{
    vdbMat4 p = vdbMatIdentity();
    p.at(0,0) = 2.0f/(x_right-x_left);
    p.at(0,3) = (x_left+x_right)/(x_left-x_right);
    p.at(1,1) = 2.0f/(y_top-y_bottom);
    p.at(1,3) = (y_bottom+y_top)/(y_bottom-y_top);
    vdbProjection(p.data);
}

void vdbOrtho(float x_left, float x_right, float y_bottom, float y_top, float z_near, float z_far)
{
    SDL_assert(false && "not implemented yet");
    vdbMat4 p = vdbMatIdentity();
    vdbProjection(p.data);
}

void vdbPerspective(float yfov, float width, float height, float z_near, float z_far)
{
    float t = 1.0f/tanf(yfov/2.0f);
    vdbMat4 p = {0};
    p.at(0,0) = t/(width/height);
    p.at(1,1) = t;
    p.at(2,2) = (z_near+z_far)/(z_near-z_far);
    p.at(3,2) = -1.0f;
    p.at(2,3) = 2.0f*z_near*z_far/(z_near-z_far);
    vdbProjection(p.data);
}

vdbVec2 vdbWindowToNDC(float xw, float yw)
{
    float xf = vdb.framebuffer_width*(xw/vdb.window_width);
    float yf = vdb.framebuffer_height*(1.0f - yw/vdb.window_height);
    float xn = -1.0f+2.0f*(xf-vdb.viewport_left)/vdb.viewport_width;
    float yn = -1.0f+2.0f*(yf-vdb.viewport_bottom)/vdb.viewport_height;
    vdbVec2 result = { xn, yn };
    return result;
}

vdbVec2 vdbNDCToWindow(float xn, float yn)
{
    float xf = vdb.viewport_left + (0.5f+0.5f*xn)*vdb.viewport_width;
    float yf = vdb.viewport_bottom + (0.5f+0.5f*yn)*vdb.viewport_height;
    float xw = vdb.window_width*(xf/vdb.framebuffer_width);
    float yw = vdb.window_height*(1.0f - yf/vdb.framebuffer_height);
    vdbVec2 result = { xw, yw };
    return result;
}

vdbVec3 vdbNDCToModel(float x_ndc, float y_ndc, float depth)
{
    // assuming projection is of the form
    // ax       bx
    //    ay    by
    //       az bz
    //       cw aw
    // (e.g. orthographic or perspective transform)

    // also assumes modelview matrix is SE3
    // (e.g. rotation and translation only)

    float ax = vdb_projection.at(0,0);
    float ay = vdb_projection.at(1,1);
    float az = vdb_projection.at(2,2);
    float bx = vdb_projection.at(0,3);
    float by = vdb_projection.at(1,3);
    float bz = vdb_projection.at(2,3);
    float cw = vdb_projection.at(3,2);
    float aw = vdb_projection.at(3,3);

    float w_clip = cw*depth + aw;
    float x_clip = x_ndc*w_clip;
    float y_clip = y_ndc*w_clip;
    // float z_clip = az*depth + bz;
    vdbVec4 view = {0};
    view.x = (x_clip-bx)/ax;
    view.y = (y_clip-by)/bx;
    view.z = depth;
    view.w = 1.0f;
    vdbVec4 model = vdbMulSE3Inverse(vdb_modelview, view);
    vdbVec3 result = { model.x, model.y, model.z };
    return result;
}

vdbVec2 vdbModelToNDC(float x, float y, float z, float w)
{
    vdbVec4 model = { x, y, z, w };
    vdbVec4 clip = vdbMul4x1(vdb_pvm, model);
    vdbVec2 ndc = { clip.x/clip.w, clip.y/clip.w };
    return ndc;
}

void vdbGetFramebufferSize(int *width, int *height) { if (width) *width = vdb.framebuffer_width; if (height) *height = vdb.framebuffer_height; }
int  vdbGetFramebufferWidth() { return vdb.framebuffer_width; }
int  vdbGetFramebufferHeight() { return vdb.framebuffer_height; }
float vdbGetFramebufferAspect() { return (float)vdb.framebuffer_width/vdb.framebuffer_height; }
