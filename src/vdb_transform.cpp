#include "_matrix.cpp"

static vdbMat4 vdb_projection = vdbMatIdentity();
static vdbMat4 vdb_modelview = vdbMatIdentity();
static vdbMat4 vdb_pvm = vdbMatIdentity();

void vdbResetTransform()
{
    vdb_projection = vdbMatIdentity();
    vdb_modelview = vdbMatIdentity();
    vdb_pvm = vdbMatIdentity();
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

void vdbMatrix(float *m)
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

void vdbPushMatrix(float *m)
{
    glMatrixMode(GL_MODELVIEW);
    if (m)
    {
        glPushMatrix();
        #ifdef VDB_MATRIX_ROW_MAJOR
        glMultMatrixf(m);
        glGetFloatv(GL_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
        #else
        glMultTransposeMatrixf(m);
        glGetFloatv(GL_TRANSPOSE_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
        #endif
        vdb_pvm = vdbMul4x4(vdb_projection, vdb_modelview);
    }
    else
    {
        glPushMatrix();
    }
}

void vdbPopMatrix()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    #ifdef VDB_MATRIX_ROW_MAJOR
    glGetFloatv(GL_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
    #else
    glGetFloatv(GL_TRANSPOSE_MODELVIEW_MATRIX, (float*)vdb_modelview.data);
    #endif
    vdb_pvm = vdbMul4x4(vdb_projection, vdb_modelview);
}

vdbMat4 vdbRotateZ(float t)
{
    vdbMat4 a = vdbMatIdentity();
    a.at(0,0) = cosf(t); a.at(0,1) = -sinf(t);
    a.at(1,0) = sinf(t); a.at(1,1) = cosf(t);
    return a;
}

vdbMat4 vdbRotateY(float t)
{
    vdbMat4 a = vdbMatIdentity();
    a.at(0,0) =  cosf(t); a.at(0,2) = sinf(t);
    a.at(2,0) = -sinf(t); a.at(2,2) = cosf(t);
    return a;
}

vdbMat4 vdbRotateX(float t)
{
    vdbMat4 a = vdbMatIdentity();
    a.at(1,1) = cosf(t); a.at(1,2) = -sinf(t);
    a.at(2,1) = sinf(t); a.at(2,2) = cosf(t);
    return a;
}

vdbMat4 vdbTranslate(float x, float y, float z)
{
    vdbMat4 a = vdbMatIdentity();
    a.at(0,3) = x;
    a.at(1,3) = y;
    a.at(2,3) = z;
    return a;
}

// M = T*Rx*Ry*Rz
void vdbMatrixEulerXYZ(float tx,float ty,float tz, float rx,float ry,float rz)
{
    vdbMat4 T = vdbTranslate(tx,ty,tz);
    vdbMat4 Rx = vdbRotateX(rx);
    vdbMat4 Ry = vdbRotateY(ry);
    vdbMat4 Rz = vdbRotateZ(rz);
    vdbMat4 M = vdbMul4x4(Ry,Rz);
            M = vdbMul4x4(Rx, M);
            M = vdbMul4x4(T, M);
    vdbMatrix(M.data);
}

// M = T*Rz*Ry*Rx
void vdbMatrixEulerZYX(float tx,float ty,float tz, float rz,float ry,float rx)
{
    vdbMat4 T = vdbTranslate(tx,ty,tz);
    vdbMat4 Rx = vdbRotateX(rx);
    vdbMat4 Ry = vdbRotateY(ry);
    vdbMat4 Rz = vdbRotateZ(rz);
    vdbMat4 M = vdbMul4x4(Ry,Rx);
            M = vdbMul4x4(Rz, M);
            M = vdbMul4x4(T, M);
    vdbMatrix(M.data);
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

void vdbPerspective(float yfov, float width, float height, float z_near, float z_far, float x_offset, float y_offset)
{
    float t = 1.0f/tanf(yfov/2.0f);
    vdbMat4 p = {0};
    p.at(0,0) = t/(width/height);
    p.at(0,2) = x_offset;
    p.at(1,1) = t;
    p.at(1,2) = y_offset;
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

int   vdbGetWindowWidth() { return vdb.window_width; }
int   vdbGetWindowHeight() { return vdb.window_height; }
int   vdbGetFramebufferWidth() { return vdb.framebuffer_width; }
int   vdbGetFramebufferHeight() { return vdb.framebuffer_height; }
float vdbGetAspectRatio() { return (float)vdb.framebuffer_width/vdb.framebuffer_height; }
