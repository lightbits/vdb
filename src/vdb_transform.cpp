namespace transform
{
    static vdbMat4 projection = vdbMatIdentity();
    static vdbMat4 view_model = vdbMatIdentity();
    static vdbMat4 pvm = vdbMatIdentity();
    static matrix_stack_t matrix_stack = {0};
    int viewport_left;
    int viewport_bottom;
    int viewport_width;
    int viewport_height;

    static void NewFrame()
    {
        projection = vdbMatIdentity();
        view_model = vdbMatIdentity();
        pvm = vdbMatIdentity();
        matrix_stack.Reset();
        vdbViewporti(0, 0, window::framebuffer_width, window::framebuffer_height);
    }
}

void vdbProjection(float *m)
{
    using namespace transform;
    if (m)
        projection = *(vdbMat4*)m;
    else
        projection = vdbMatIdentity();
    pvm = vdbMul4x4(projection, view_model);
}

void vdbGetProjection(float *m)
{
    assert(m);
    *(vdbMat4*)m = transform::projection;
}

void vdbGetPVM(float *m)
{
    assert(m);
    *(vdbMat4*)m = transform::pvm;
}

void vdbGetMatrix(float *m)
{
    assert(m);
    *(vdbMat4*)m = transform::view_model;
}

void vdbMultMatrix(float *m)
{
    using namespace transform;
    if (m)
    {
        matrix_stack.Multiply(*(vdbMat4*)m);
        view_model = matrix_stack.Top();
        pvm = vdbMul4x4(projection, view_model);
    }
}

void vdbLoadMatrix(float *m)
{
    using namespace transform;
    if (m)
        matrix_stack.Load(*(vdbMat4*)m);
    else
        matrix_stack.LoadIdentity();
    view_model = matrix_stack.Top();
    pvm = vdbMul4x4(projection, view_model);
}

void vdbPushMatrix()
{
    using namespace transform;
    matrix_stack.Push();
    view_model = matrix_stack.Top();
}

void vdbPopMatrix()
{
    using namespace transform;
    matrix_stack.Pop();
    view_model = matrix_stack.Top();
    pvm = vdbMul4x4(projection, view_model);
}

void vdbTranslate(float x, float y, float z) { vdbMultMatrix(vdbMatTranslate(x,y,z).data); }
void vdbRotateXYZ(float x, float y, float z) { vdbMultMatrix(vdbMatRotateXYZ(x,y,z).data); }
void vdbRotateZYX(float z, float y, float x) { vdbMultMatrix(vdbMatRotateZYX(z,y,x).data); }

int vdbGetWindowWidth() { return window::window_width; }
int vdbGetWindowHeight() { return window::window_height; }

int vdbGetFramebufferWidth()
{
    using namespace render_texture;
    if (current_render_texture)
        return current_render_texture->width;
    else return window::framebuffer_width;
}

int vdbGetFramebufferHeight()
{
    using namespace render_texture;
    if (current_render_texture)
        return current_render_texture->height;
    else return window::framebuffer_height;
}

float vdbGetAspectRatio()
{
    return (float)vdbGetFramebufferWidth()/vdbGetFramebufferHeight();
}

void vdbViewporti(int left, int bottom, int width, int height)
{
    glViewport(left, bottom, (GLsizei)width, (GLsizei)height);
    transform::viewport_left = left;
    transform::viewport_bottom = bottom;
    transform::viewport_width = width;
    transform::viewport_height = height;
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
    using namespace transform;
    float xf = vdbGetFramebufferWidth()*(xw/vdbGetWindowWidth());
    float yf = vdbGetFramebufferHeight()*(1.0f - yw/vdbGetWindowHeight());
    float xn = -1.0f+2.0f*(xf-viewport_left)/viewport_width;
    float yn = -1.0f+2.0f*(yf-viewport_bottom)/viewport_height;
    vdbVec2 result(xn,yn);
    return result;
}

vdbVec2 vdbNDCToWindow(float xn, float yn)
{
    using namespace transform;
    float xf = viewport_left + (0.5f+0.5f*xn)*viewport_width;
    float yf = viewport_bottom + (0.5f+0.5f*yn)*viewport_height;
    float xw = vdbGetWindowWidth()*(xf/vdbGetFramebufferWidth());
    float yw = vdbGetWindowHeight()*(1.0f - yf/vdbGetFramebufferHeight());
    vdbVec2 result(xw,yw);
    return result;
}

vdbVec3 vdbNDCToModel(float x_ndc, float y_ndc, float depth)
{
    using namespace transform;

    // assuming projection is of the form
    // ax       bx
    //    ay    by
    //       az bz
    //       cw aw
    // (e.g. orthographic or perspective transform)

    // also assumes modelview matrix is SE3
    // (e.g. rotation and translation only)

    float ax = projection(0,0);
    float ay = projection(1,1);
    // float az = projection(2,2);
    float bx = projection(0,3);
    float by = projection(1,3);
    // float bz = projection(2,3);
    float cw = projection(3,2);
    float aw = projection(3,3);

    float w_clip = cw*depth + aw;
    float x_clip = x_ndc*w_clip;
    float y_clip = y_ndc*w_clip;
    // float z_clip = az*depth + bz;
    vdbVec4 view(0,0,0,0);
    view.x = (x_clip-bx)/ax;
    view.y = (y_clip-by)/ay;
    view.z = depth;
    view.w = 1.0f;
    vdbVec4 model = vdbMulSE3Inverse(view_model, view);
    vdbVec3 result(model.x,model.y,model.z);
    return result;
}

vdbVec2 vdbModelToNDC(float x, float y, float z, float w)
{
    vdbVec4 model(x,y,z,w);
    vdbVec4 clip = vdbMul4x1(transform::pvm, model);
    vdbVec2 ndc(clip.x/clip.w, clip.y/clip.w);
    return ndc;
}
