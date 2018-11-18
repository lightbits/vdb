void vdbCameraTurntable(vdbVec3 look_at, float init_radius)
{
    static float angle_x = 0.0f;
    static float angle_y = 0.0f;
    static float radius = init_radius;
    static float ref_angle_x = 0.0f;
    static float ref_angle_y = 0.0f;
    static float ref_radius = init_radius;

    // todo: multiply with current modelview

    static float K_ref = 50.0f;
    static float K_track = 10.0f;
    static float K_radius = 5.0f;

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
        ref_angle_x += K_ref*dy*1.0f/60.0f;
        ref_angle_y -= K_ref*dx*1.0f/60.0f;
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
        if (!vdbIsMouseLeftDown())
            dragging = false;
    }

    ref_radius -= ref_radius*K_radius*vdbGetMouseWheel()*1.0f/60.0f;

    angle_x += K_track*(ref_angle_x - angle_x)*1.0f/60.0f;
    angle_y += K_track*(ref_angle_y - angle_y)*1.0f/60.0f;
    radius += K_track*(ref_radius - radius)*1.0f/60.0f;

    #if 0
    ImGui::Begin("Camera controls");
    {
        ImGui::SliderFloat("K_ref", &K_ref, 0.1f, 100.0f);
        ImGui::SliderFloat("K_track", &K_track, 0.1f, 100.0f);
        ImGui::SliderFloat("K_radius", &K_radius, 0.1f, 100.0f);
    }
    ImGui::End();
    #endif

    vdbTranslate(0.0f, 0.0f, -radius);
    vdbRotateXYZ(-angle_x, -angle_y, 0.0f);
}

