#include "_matrix.cpp"

static vdbMat4 vdb_projection = vdbMatIdentity();
static vdbMat4 vdb_modelview = vdbMatIdentity();
static vdbMat4 vdb_pvm = vdbMatIdentity();

#if VDB_USE_FIXED_FUNCTION_PIPELINE==1
// This path uses the fixed-function pipeline of legacy OpenGL.
// It is available only in compatibility profiles of OpenGL, which
// itself is not available on certain drivers (Mesa, for one).

void vdbResetTransform()
{
    vdb_projection = vdbMatIdentity();
    vdb_modelview = vdbMatIdentity();
    vdb_pvm = vdbMatIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    CheckGLError();
}

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

void vdbGetMatrix(float *m)
{
    assert(m && "pointer passed to vdbGetMatrix was NULL");
    glGetFloatv(GL_MODELVIEW_MATRIX, m);
}

void vdbMultMatrix(float *m)
{
    assert(m && "pointer passed to vdbMultMatrix was NULL");
    if (m)
    {
        glMatrixMode(GL_MODELVIEW);
        #ifdef VDB_MATRIX_ROW_MAJOR
        glMultMatrixf(m);
        glGetFloatv(GL_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
        #else
        glMultTransposeMatrixf(m);
        glGetFloatv(GL_TRANSPOSE_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
        #endif
        vdb_pvm = vdbMul4x4(vdb_projection, vdb_modelview);
    }
}

void vdbLoadMatrix(float *m)
{
    glMatrixMode(GL_MODELVIEW);
    if (m)
    {
        #ifdef VDB_MATRIX_ROW_MAJOR
        glLoadMatrixf(m);
        glGetFloatv(GL_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
        #else
        glLoadTransposeMatrixf(m);
        glGetFloatv(GL_TRANSPOSE_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
        #endif
    }
    else
    {
        vdb_modelview = vdbMatIdentity();
        glLoadIdentity();
    }
    vdb_pvm = vdbMul4x4(vdb_projection, vdb_modelview);
}

static int vdb_push_pop_matrix_index = 0;

void vdbPushMatrix()
{
    vdb_push_pop_matrix_index++;
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

void vdbPopMatrix()
{
    vdb_push_pop_matrix_index--;
    assert(vdb_push_pop_matrix_index >= 0 && "Mismatched vdb{Push/Pop}Matrix calls");
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    #ifdef VDB_MATRIX_ROW_MAJOR
    glGetFloatv(GL_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
    #else
    glGetFloatv(GL_TRANSPOSE_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
    #endif
    vdb_pvm = vdbMul4x4(vdb_projection, vdb_modelview);
}

#else

#error "Not implemented"

#endif

void vdbTranslate(float x, float y, float z) { vdbMultMatrix(vdbMatTranslate(x,y,z).data); }
void vdbRotateXYZ(float x, float y, float z) { vdbMultMatrix(vdbMatRotateXYZ(x,y,z).data); }
void vdbRotateZYX(float z, float y, float x) { vdbMultMatrix(vdbMatRotateZYX(z,y,x).data); }

int vdbGetWindowWidth() { return vdb.window_width; }
int vdbGetWindowHeight() { return vdb.window_height; }

int vdbGetFramebufferWidth()
{
    if (vdb.current_render_texture) return vdb.current_render_texture->width;
    else return vdb.framebuffer_width;
}

int vdbGetFramebufferHeight()
{
    if (vdb.current_render_texture) return vdb.current_render_texture->height;
    else return vdb.framebuffer_height;
}

float vdbGetAspectRatio()
{
    return (float)vdbGetFramebufferWidth()/vdbGetFramebufferHeight();
}

void vdbViewporti(int left, int bottom, int width, int height)
{
    glViewport(left, bottom, (GLsizei)width, (GLsizei)height);
    vdb.viewport_left = left;
    vdb.viewport_bottom = bottom;
    vdb.viewport_width = width;
    vdb.viewport_height = height;
}

void vdbViewport(float left, float bottom, float width, float height)
{
    int fb_width = vdbGetFramebufferWidth();
    int fb_height = vdbGetFramebufferHeight();
    vdbViewporti((int)(left*fb_width), (int)(bottom*fb_height),
                 (int)(width*fb_width), (int)(height*fb_height));
}

void vdbOrtho(float x_left, float x_right, float y_bottom, float y_top)
{
    vdbMat4 p = vdbMatIdentity();
    p(0,0) = 2.0f/(x_right-x_left);
    p(0,3) = (x_left+x_right)/(x_left-x_right);
    p(1,1) = 2.0f/(y_top-y_bottom);
    p(1,3) = (y_bottom+y_top)/(y_bottom-y_top);
    vdbProjection(p.data);
}

void vdbOrtho(float x_left, float x_right, float y_bottom, float y_top, float z_near, float z_far)
{
    assert(false && "not implemented yet");
    vdbMat4 p = vdbMatIdentity();
    vdbProjection(p.data);
}

void vdbPerspective(float yfov, float z_near, float z_far, float x_offset, float y_offset)
{
    float t = 1.0f/tanf(yfov/2.0f);
    vdbMat4 p = {0};
    p(0,0) = t/(vdbGetAspectRatio());
    p(0,2) = x_offset;
    p(1,1) = t;
    p(1,2) = y_offset;
    p(2,2) = (z_near+z_far)/(z_near-z_far);
    p(3,2) = -1.0f;
    p(2,3) = 2.0f*z_near*z_far/(z_near-z_far);
    vdbProjection(p.data);
}

vdbVec2 vdbWindowToNDC(float xw, float yw)
{
    float xf = vdb.framebuffer_width*(xw/vdb.window_width);
    float yf = vdb.framebuffer_height*(1.0f - yw/vdb.window_height);
    float xn = -1.0f+2.0f*(xf-vdb.viewport_left)/vdb.viewport_width;
    float yn = -1.0f+2.0f*(yf-vdb.viewport_bottom)/vdb.viewport_height;
    vdbVec2 result(xn,yn);
    return result;
}

vdbVec2 vdbNDCToWindow(float xn, float yn)
{
    float xf = vdb.viewport_left + (0.5f+0.5f*xn)*vdb.viewport_width;
    float yf = vdb.viewport_bottom + (0.5f+0.5f*yn)*vdb.viewport_height;
    float xw = vdb.window_width*(xf/vdb.framebuffer_width);
    float yw = vdb.window_height*(1.0f - yf/vdb.framebuffer_height);
    vdbVec2 result(xw,yw);
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

    float ax = vdb_projection(0,0);
    float ay = vdb_projection(1,1);
    // float az = vdb_projection(2,2);
    float bx = vdb_projection(0,3);
    float by = vdb_projection(1,3);
    // float bz = vdb_projection(2,3);
    float cw = vdb_projection(3,2);
    float aw = vdb_projection(3,3);

    float w_clip = cw*depth + aw;
    float x_clip = x_ndc*w_clip;
    float y_clip = y_ndc*w_clip;
    // float z_clip = az*depth + bz;
    vdbVec4 view(0,0,0,0);
    view.x = (x_clip-bx)/ax;
    view.y = (y_clip-by)/ay;
    view.z = depth;
    view.w = 1.0f;
    vdbVec4 model = vdbMulSE3Inverse(vdb_modelview, view);
    vdbVec3 result(model.x,model.y,model.z);
    return result;
}

vdbVec2 vdbModelToNDC(float x, float y, float z, float w)
{
    vdbVec4 model(x,y,z,w);
    vdbVec4 clip = vdbMul4x1(vdb_pvm, model);
    vdbVec2 ndc(clip.x/clip.w, clip.y/clip.w);
    return ndc;
}
