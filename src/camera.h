enum camera_aspect_mode_t { VDB_STRETCH_TO_FIT, VDB_EQUAL_AXES };

void vdbCamera2D()
{
    const float dt = 1.0f/60.0f;

    GetFrameSettings()->camera.planar.dirty = true;
    float scroll_sensitivity = settings.camera.scroll_sensitivity;
    float mouse_sensitivity = settings.camera.mouse_sensitivity;
    float &zoom = GetFrameSettings()->camera.planar.zoom;
    float &angle = GetFrameSettings()->camera.planar.angle;
    float &position_x = GetFrameSettings()->camera.planar.position.x;
    float &position_y = GetFrameSettings()->camera.planar.position.y;

    // zooming
    #if 1
    // this version keeps the point underneath the cursor fixed
    {
        float W = (float)vdbGetFramebufferWidth();
        float H = (float)vdbGetFramebufferHeight();
        float A = W < H ? W : H;
        if (vdbGetMouseWheel() < 0.0f)
        {
            float next_zoom = zoom*1.2f;
            position_x += W/A*vdbGetMousePosNDC().x*(next_zoom - zoom);
            position_y += H/A*vdbGetMousePosNDC().y*(next_zoom - zoom);
            zoom = next_zoom;
        }
        else if (vdbGetMouseWheel() > 0.0f)
        {
            float next_zoom = zoom/1.2f;
            position_x += W/A*vdbGetMousePosNDC().x*(next_zoom - zoom);
            position_y += H/A*vdbGetMousePosNDC().y*(next_zoom - zoom);
            zoom = next_zoom;
        }
    }
    #else
    zoom -= scroll_sensitivity*vdbGetMouseWheel()*zoom*dt;
    #endif

    // rotation
    {
        const float pi = 3.14159265359f;

        float aspect = vdbGetAspectRatio();
        static float last_mouse_angle = 0.0f;
        float mouse_dx = vdbGetMousePosNDC().x*aspect;
        float mouse_dy = vdbGetMousePosNDC().y;
        float mouse_angle = atan2f(mouse_dy, mouse_dx);
        float delta_angle = mouse_angle - last_mouse_angle;
        last_mouse_angle = mouse_angle;
        if (vdbIsMouseRightDown() && fabsf(delta_angle) < 1.9f*pi)
        {
            angle += mouse_sensitivity*delta_angle*dt;
        }

        if (angle < 0.0f) angle += 2.0f*pi;
        if (angle > 2.0f*pi) angle -= 2.0f*pi;

        const float th = 0.05f;
        if      (fabsf(angle - 0.0f*pi) < th) angle = 0.0f*pi;
        else if (fabsf(angle - 0.5f*pi) < th) angle = 0.5f*pi;
        else if (fabsf(angle - 1.0f*pi) < th) angle = 1.0f*pi;
        else if (fabsf(angle - 1.5f*pi) < th) angle = 1.5f*pi;
    }

    // translation
    {
        float aspect = vdbGetAspectRatio();
        static float last_mouse_x = 0.0f, last_mouse_y = 0.0f;
        float mouse_x = vdbGetMousePosNDC().x*aspect;
        float mouse_y = vdbGetMousePosNDC().y;
        float dx = mouse_x - last_mouse_x;
        float dy = mouse_y - last_mouse_y;
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
        if (vdbIsMouseLeftDown())
        {
            position_x += mouse_sensitivity*zoom*dx*dt;
            position_y += mouse_sensitivity*zoom*dy*dt;
        }
    }

    {
        float W = (float)vdbGetFramebufferWidth();
        float H = (float)vdbGetFramebufferHeight();
        float A = W < H ? W : H;
        vdbOrtho(-zoom*W/A, zoom*W/A, -zoom*H/A, zoom*H/A);
    }
    {
        vdbMat4 M = vdbMatTranslate(position_x, position_y, 0.0f)*vdbMatRotateXYZ(0.0f,0.0f,angle);
        vdbLoadMatrix(M.data);
    }
}

void vdbCameraTrackball()
{
    const float dt = 1.0f/60.0f; // todo: expose API vdbGetFrameDelta()

    auto &cs = settings.camera;
    GetFrameSettings()->camera.trackball.dirty = true;
    vdbMat4 &R0 = GetFrameSettings()->camera.trackball.R;
    vdbVec4 &T = GetFrameSettings()->camera.trackball.T;
    float &zoom = GetFrameSettings()->camera.trackball.zoom;

    vdbMat4 R = R0;

    float move_speed = cs.move_speed_normal;
    if (vdbIsKeyDown(VDB_KEY_LSHIFT)) move_speed = cs.move_speed_slow;

    // zooming
    zoom -= cs.scroll_sensitivity*vdbGetMouseWheel()*zoom*dt;

    // translation
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        if (vdbIsKeyDown(VDB_KEY_A))     x = -move_speed*zoom;
        if (vdbIsKeyDown(VDB_KEY_D))     x = +move_speed*zoom;
        if (vdbIsKeyDown(VDB_KEY_W))     z = -move_speed*zoom;
        if (vdbIsKeyDown(VDB_KEY_S))     z = +move_speed*zoom;
        if (vdbIsKeyDown(VDB_KEY_LCTRL)) y = -move_speed*zoom;
        if (vdbIsKeyDown(VDB_KEY_SPACE)) y = +move_speed*zoom;
        vdbVec4 in_camera_vel = vdbVec4(x,y,z,0.0f);
        vdbVec4 in_world_vel = vdbMulTranspose4x1(R, in_camera_vel);
        T = T + in_world_vel*dt;
    }

    float radius = 1.0f;
    float aspect = vdbGetAspectRatio();

    static bool dragging = false;
    static float mouse_x_start = 0.0f, mouse_y_start = 0.0f;
    float mouse_x = vdbGetMousePosNDC().x*aspect;
    float mouse_y = vdbGetMousePosNDC().y;
    if (!dragging && vdbIsMouseLeftDown())
    {
        mouse_x_start = mouse_x;
        mouse_y_start = mouse_y;
        dragging = true;
    }
    if (dragging)
    {
        float mouse_x_end = mouse_x;
        float mouse_y_end = mouse_y;

        // this has a problem when the two points are colinear
        // fixed by clamping angle

        // theta = a * length(u,v)/r sphere projection
        // x = r sin(theta) cos(phi)
        // y = r sin(theta) sin(phi)
        // z = r cos(theta)
        float u1 = mouse_x_start;
        float v1 = mouse_y_start;
        float u2 = mouse_x_end;
        float v2 = mouse_y_end;
        float r = radius;
        float t1 = (3.14f/3.0f)*sqrtf(u1*u1 + v1*v1)/r;
        float t2 = (3.14f/3.0f)*sqrtf(u2*u2 + v2*v2)/r;
        // clamp angle
        if (t1 > 3.14f/3.0f) t1 = 3.14f/3.0f;
        if (t2 > 3.14f/3.0f) t2 = 3.14f/3.0f;
        float h1 = atan2f(v1,u1);
        float h2 = atan2f(v2,u2);
        float x1 = r*sinf(t1)*cosf(h1);
        float y1 = r*sinf(t1)*sinf(h1);
        float z1 = r*cosf(t1);

        float x2 = r*sinf(t2)*cosf(h2);
        float y2 = r*sinf(t2)*sinf(h2);
        float z2 = r*cosf(t2);

        if (x2 != x1 || y2 != y1)
        {
            vdbVec3 p1 = vdbVec3(x1, y1, z1);
            vdbVec3 p2 = vdbVec3(x2, y2, z2);
            vdbVec3 c = vdbVecCross(p1, p2);
            float l = vdbVecLength(p1)*vdbVecLength(p2);
            float sin_t = vdbVecLength(c)/l;
            float cos_t = vdbVecDot(p1,p2)/l;
            vdbVec3 w_sin_t = c/l;
            vdbMat4 W = vdbMatSkew(w_sin_t/sin_t);
            vdbMat4 R_delta = vdbMatIdentity() + sin_t*W + (1.0f - cos_t)*W*W;
            R = R_delta*R0;
        }
        if (!vdbIsMouseLeftDown())
        {
            R0 = vdbMatOrthogonalize(R);
            dragging = false;
        }
    }
    {
        vdbVec4 Tc = -(R*T);
        vdbMat4 M = vdbMatTranslate(Tc.x,Tc.y,Tc.z - zoom)*R;
        vdbLoadMatrix(M.data);
    }
}

void vdbCameraTurntable()
{
    GetFrameSettings()->camera.turntable.dirty = true;
    float scroll_sensitivity = settings.camera.scroll_sensitivity;
    float mouse_sensitivity = settings.camera.mouse_sensitivity;
    float &angle_x = GetFrameSettings()->camera.turntable.angle_x;
    float &angle_y = GetFrameSettings()->camera.turntable.angle_y;
    float &radius = GetFrameSettings()->camera.turntable.radius;
    const float dt = 1.0f/60.0f;

    // zooming
    radius -= scroll_sensitivity*vdbGetMouseWheel()*radius*dt;

    float aspect = vdbGetAspectRatio();
    static bool dragging = false;
    static float last_mouse_x = 0.0f, last_mouse_y = 0.0f;
    float mouse_x = vdbGetMousePosNDC().x*aspect;
    float mouse_y = vdbGetMousePosNDC().y;
    if (!dragging && vdbIsMouseLeftDown())
    {
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
        dragging = true;
    }
    if (dragging)
    {
        float dx = vdbGetMousePosNDC().x*aspect - last_mouse_x;
        float dy = vdbGetMousePosNDC().y - last_mouse_y;
        angle_x += mouse_sensitivity*dy*dt;
        angle_y -= mouse_sensitivity*dx*dt;
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
        if (!vdbIsMouseLeftDown())
            dragging = false;
    }

    {
        vdbMat4 M = vdbMatTranslate(0.0f, 0.0f, -radius)*vdbMatRotateXYZ(-angle_x, -angle_y, 0.0f);
        vdbLoadMatrix(M.data);
    }
}

