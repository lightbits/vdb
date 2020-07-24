namespace immediate_util
{
    static int note_index;
    static float note_align_x;
    static float note_align_y;
    static char temp_buffer[1024*3 + 1];
    static void BeginFrame()
    {
        note_index = 0;
        note_align_x = 0.0f;
        note_align_y = 0.0f;
    }
}

void vdbNoteV(float x, float y, const char *fmt, va_list args)
{
    using namespace immediate_util;
    vdbVec2 ndc = vdbModelToNDC(x,y,0.0f,1.0f);
    vdbVec2 win = vdbNDCToWindow(ndc.x,ndc.y);
    ImFormatString(temp_buffer, sizeof(temp_buffer), "vdb_tooltip_%d", note_index);
    ImGui::SetNextWindowPos(ImVec2(win.x, win.y), 0, ImVec2(note_align_x, note_align_y));
    ImGui::Begin(temp_buffer, 0, ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextV(fmt, args);
    ImGui::End();
    note_index++;
}

void vdbNote(float x, float y, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vdbNoteV(x, y, fmt, args);
    va_end(args);
}

void vdbNoteAlign(float x, float y)
{
    immediate_util::note_align_x = x;
    immediate_util::note_align_y = y;
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
    vdbBeginTriangles();
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

void vdbLineCircle_(float x, float y, float radius)
{
    int segments = imm.state.point_segments;
    const float two_pi = 6.28318530718f;
    for (int i = 0; i < segments; i++)
    {
        int i1 = i;
        int i2 = (i + 1) % segments;
        float t0 = two_pi*i1/(float)segments;
        float t1 = two_pi*i2/(float)segments;
        vdbVertex(x + radius*cosf(t0),y + radius*sinf(t0));
        vdbVertex(x + radius*cosf(t1),y + radius*sinf(t1));
    }
}
void vdbLineCircle(float x, float y, float radius)
{
    vdbBeginLines();
    vdbLineCircle_(x, y, radius);
    vdbEnd();
}

void vdbFillCircle_(float x, float y, float radius)
{
    int segments = imm.state.point_segments;
    const float two_pi = 6.28318530718f;
    for (int i = 0; i < segments; i++)
    {
        int i1 = i;
        int i2 = (i + 1) % segments;
        float t0 = two_pi*i1/(float)segments;
        float t1 = two_pi*i2/(float)segments;
        vdbVertex(x, y);
        vdbVertex(x + radius*cosf(t0),y + radius*sinf(t0));
        vdbVertex(x + radius*cosf(t1),y + radius*sinf(t1));
    }
}
void vdbFillCircle(float x, float y, float radius)
{
    vdbBeginTriangles();
    vdbFillCircle_(x, y, radius);
    vdbEnd();
}

void vdbFillTexturedRect_(float x, float y, float w, float h)
{
    vdbTexel(0,0); vdbVertex(x, y);
    vdbTexel(1,0); vdbVertex(x+w, y);
    vdbTexel(1,1); vdbVertex(x+w, y+h);
    vdbTexel(1,1); vdbVertex(x+w, y+h);
    vdbTexel(0,1); vdbVertex(x, y+h);
    vdbTexel(0,0); vdbVertex(x, y);
}
void vdbFillTexturedRect(float x, float y, float w, float h)
{
    vdbBeginTriangles();
    vdbFillTexturedRect_(x, y, w, h);
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

void vdbFillArc_(vdbVec3 base, vdbVec3 p1, vdbVec3 p2)
{
    int segments = imm.state.point_segments;
    float r1 = vdbVecLength(p1);
    float r2 = vdbVecLength(p2);
    for (int i = 0; i < segments; i++)
    {
        float t1 = (float)(i+0)/segments;
        float t2 = (float)(i+1)/segments;
        float rq1 = r1 + (r2-r1)*t1;
        float rq2 = r1 + (r2-r1)*t2;
        vdbVec3 q1 = vdbVecNormalize(p1 + (p2-p1)*t1)*rq1;
        vdbVec3 q2 = vdbVecNormalize(p1 + (p2-p1)*t2)*rq2;
        vdbVertex(0,0,0);
        vdbVertex(q1.x, q1.y, q1.z);
        vdbVertex(q2.x, q2.y, q2.z);
    }
}
void vdbFillArc(vdbVec3 base, vdbVec3 p1, vdbVec3 p2)
{
    vdbBeginTriangles();
    vdbFillArc_(base, p1, p2);
    vdbEnd();
}

// void vdbDrawAxes(vdbVec3 center, vdbVec3 x, vdbVec3 y, vdbVec3 z, float scale=1.0f);
// void vdbDrawTextV(float x, float y, const char *fmt, va_list args);
// void vdbDrawText(float x, float y, const char *fmt, ...);
