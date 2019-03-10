namespace immediate_util
{
    static int note_index;
    static void NewFrame()
    {
        note_index = 0;
    }
}

void vdbNoteV(float x, float y, const char *fmt, va_list args)
{
    // Transform position to window coordinates
    vdbVec2 ndc = vdbModelToNDC(x,y,0.0f,1.0f);
    vdbVec2 win = vdbNDCToWindow(ndc.x,ndc.y);

    char name[1024];
    sprintf(name, "vdb_tooltip_%d", immediate_util::note_index);
    ImGui::SetNextWindowPos(ImVec2(win.x, win.y));
    ImGui::Begin(name, 0, ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextV(fmt, args);
    ImGui::End();
    immediate_util::note_index++;
}
void vdbNote(float x, float y, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vdbNoteV(x, y, fmt, args);
    va_end(args);
}

void vdbFillRect_(float x, float y, float w, float h)
{
    vdbVertex(x, y);
    vdbVertex(x+w, y);
    vdbVertex(x+w, y+h);
    vdbVertex(x+w, y+h);
    vdbVertex(x, y+h);
    vdbVertex(x, y);
}
void vdbFillRect(float x, float y, float w, float h)
{
    vdbTriangles();
    vdbFillRect_(x, y, w, h);
    vdbEnd();
}

void vdbLineRect_(float x, float y, float w, float h)
{
    vdbVertex(x, y); vdbVertex(x+w, y);
    vdbVertex(x+w, y); vdbVertex(x+w, y+h);
    vdbVertex(x+y, y+h); vdbVertex(x, y+h);
    vdbVertex(x, y+h); vdbVertex(x, y);
}
void vdbLineRect(float x, float y, float w, float h)
{
    vdbBeginLines();
    vdbFillRect_(x, y, w, h);
    vdbEnd();
}

void vdbLineCircle_(float x, float y, float radius, int segments)
{
    const float two_pi = 6.28318530718f;
    for (int i = 0; i < segments; i++)
    {
        float t0 = two_pi*i/(float)segments;
        float t1 = two_pi*(i+1)/(float)segments;
        vdbVertex(x + radius*cosf(t0),y + radius*sinf(t0));
        vdbVertex(x + radius*cosf(t1),y + radius*sinf(t1));
    }
}
void vdbLineCircle(float x, float y, float radius, int segments)
{
    vdbBeginLines();
    vdbLineCircle_(x, y, radius, segments);
    vdbEnd();
}

void vdbLineGrid_(float x_min, float x_max, float y_min, float y_max, int n)
{
    for (int i = 0; i <= n; i++)
    {
        float x = x_min + (x_max-x_min)*(float)i/n;
        float y = y_min + (y_max-y_min)*(float)i/n;
        vdbVertex(x_min, y); vdbVertex(x_max, y);
        vdbVertex(x, y_min); vdbVertex(x, y_max);
    }
}
void vdbLineGrid(float x_min, float x_max, float y_min, float y_max, int n)
{
    vdbBeginLines();
    vdbLineGrid_(x_min, x_max, y_min, y_max, n);
    vdbEnd();
}

void vdbLineCube_(float wx, float wy, float wz)
{
    float x2 = wx/2.0f;
    float y2 = wy/2.0f;
    float z2 = wz/2.0f;
    vdbVertex(-x2, -y2, -z2); vdbVertex(+x2, -y2, -z2);
    vdbVertex(+x2, -y2, -z2); vdbVertex(+x2, +y2, -z2);
    vdbVertex(+x2, +y2, -z2); vdbVertex(-x2, +y2, -z2);
    vdbVertex(-x2, +y2, -z2); vdbVertex(-x2, -y2, -z2);

    vdbVertex(-x2, -y2, +z2); vdbVertex(+x2, -y2, +z2);
    vdbVertex(+x2, -y2, +z2); vdbVertex(+x2, +y2, +z2);
    vdbVertex(+x2, +y2, +z2); vdbVertex(-x2, +y2, +z2);
    vdbVertex(-x2, +y2, +z2); vdbVertex(-x2, -y2, +z2);

    vdbVertex(-x2, -y2, -z2); vdbVertex(-x2, -y2, +z2);
    vdbVertex(+x2, -y2, -z2); vdbVertex(+x2, -y2, +z2);
    vdbVertex(+x2, +y2, -z2); vdbVertex(+x2, +y2, +z2);
    vdbVertex(-x2, +y2, -z2); vdbVertex(-x2, +y2, +z2);
}
void vdbLineCube(float wx, float wy, float wz)
{
    vdbBeginLines();
    vdbLineCube_(wx, wy, wz);
    vdbEnd();
}

void vdbLineCube_(vdbVec3 p_min, vdbVec3 p_max)
{
    vdbVertex(p_min.x, p_min.y, p_min.z); vdbVertex(p_max.x, p_min.y, p_min.z);
    vdbVertex(p_max.x, p_min.y, p_min.z); vdbVertex(p_max.x, p_max.y, p_min.z);
    vdbVertex(p_max.x, p_max.y, p_min.z); vdbVertex(p_min.x, p_max.y, p_min.z);
    vdbVertex(p_min.x, p_max.y, p_min.z); vdbVertex(p_min.x, p_min.y, p_min.z);

    vdbVertex(p_min.x, p_min.y, p_max.z); vdbVertex(p_max.x, p_min.y, p_max.z);
    vdbVertex(p_max.x, p_min.y, p_max.z); vdbVertex(p_max.x, p_max.y, p_max.z);
    vdbVertex(p_max.x, p_max.y, p_max.z); vdbVertex(p_min.x, p_max.y, p_max.z);
    vdbVertex(p_min.x, p_max.y, p_max.z); vdbVertex(p_min.x, p_min.y, p_max.z);

    vdbVertex(p_min.x, p_min.y, p_min.z); vdbVertex(p_min.x, p_min.y, p_max.z);
    vdbVertex(p_max.x, p_min.y, p_min.z); vdbVertex(p_max.x, p_min.y, p_max.z);
    vdbVertex(p_max.x, p_max.y, p_min.z); vdbVertex(p_max.x, p_max.y, p_max.z);
    vdbVertex(p_min.x, p_max.y, p_min.z); vdbVertex(p_min.x, p_max.y, p_max.z);
}
void vdbLineCube(vdbVec3 p_min, vdbVec3 p_max)
{
    vdbBeginLines();
    vdbLineCube_(p_min, p_max);
    vdbEnd();
}

void vdbFillArc_(vdbVec3 base, vdbVec3 p1, vdbVec3 p2, int n)
{
    float r1 = vdbVecLength(p1);
    float r2 = vdbVecLength(p2);
    for (int i = 0; i < n; i++)
    {
        float t1 = (float)(i+0)/n;
        float t2 = (float)(i+1)/n;
        float rq1 = r1 + (r2-r1)*t1;
        float rq2 = r1 + (r2-r1)*t2;
        vdbVec3 q1 = vdbVecNormalize(p1 + (p2-p1)*t1)*rq1;
        vdbVec3 q2 = vdbVecNormalize(p1 + (p2-p1)*t2)*rq2;
        vdbVertex(0,0,0); vdbVertex(q1); vdbVertex(q2);
    }
}
void vdbFillArc(vdbVec3 base, vdbVec3 p1, vdbVec3 p2, int n)
{
    vdbTriangles();
    vdbFillArc_(base, p1, p2, n);
    vdbEnd();
}

// void vdbDrawAxes(vdbVec3 center, vdbVec3 x, vdbVec3 y, vdbVec3 z, float scale=1.0f);
// void vdbDrawTextV(float x, float y, const char *fmt, va_list args);
// void vdbDrawText(float x, float y, const char *fmt, ...);
