void vdbInverseColor(bool enable)
{
    if (enable)
    {
        glLogicOp(GL_XOR);
        glEnable(GL_COLOR_LOGIC_OP);
        glColor4ub(0x80, 0x80, 0x80, 0x00);
    }
    else
    {
        glDisable(GL_COLOR_LOGIC_OP);
    }
}
void vdbNoteV(float x, float y, const char *fmt, va_list args)
{
    // Transform position to window coordinates
    vdbVec2 ndc = vdbModelToNDC(x,y,0.0f,1.0f);
    vdbVec2 win = vdbNDCToWindow(ndc.x,ndc.y);

    // Clamp tooltip to window
    // (Doesn't work yet)
    #if 0
    {
        char text[1024];
        sprintf(text, fmt, args);

        ImVec2 size = ImGui::CalcTextSize(text);

        if (x_win + size.x + 20.0f > vdb.window_w)
            x_win = vdb.window_w - size.x - 20.0f;

        if (y_win + size.y + 20.0f > vdb.window_h)
            y_win = vdb.window_h - size.y - 20.0f;
    }
    #endif

    char name[1024];
    sprintf(name, "vdb_tooltip_%d", vdb.note_index);
    ImGui::SetNextWindowPos(ImVec2(win.x, win.y));
    ImGui::Begin(name, 0, ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextV(fmt, args);
    ImGui::End();
    vdb.note_index++;
}
void vdbNote(float x, float y, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vdbNoteV(x, y, fmt, args);
    va_end(args);
}
void vdbClearColor(float r, float g, float b, float a)
{
    glClearColor(r,g,b,a);
    glClear(GL_COLOR_BUFFER_BIT);
}
void vdbClearDepth(float d)
{
    glClearDepth(d);
    glClear(GL_DEPTH_BUFFER_BIT);
}
void vdbLineWidth(float width) { glLineWidth(width); }
void vdbBeginLines() { glBegin(GL_LINES); }
void vdbLines(float width) { glLineWidth(width); glBegin(GL_LINES); }
void vdbLineSize(float width) { glLineWidth(width); }
void vdbBeginPoints() { glBegin(GL_POINTS); }
void vdbPoints(float radius) { glPointSize(radius); glBegin(GL_POINTS); }
void vdbTriangles() { glBegin(GL_TRIANGLES); }
void vdbEnd() { glEnd(); }

void vdbVertex(float x, float y, float z, float w) { glVertex4f(x,y,z,w); }
void vdbColor(float r, float g, float b, float a)  { glColor4f(r,g,b,a); }
void vdbTexel(float u, float v)                    { glTexCoord2f(u,v); }

void vdbVertex(vdbVec3 v, float w) { glVertex4f(v.x, v.y, v.z, w); }
void vdbVertex(vdbVec4 v)          { glVertex4f(v.x, v.y, v.z, v.w); }
void vdbColor(vdbVec3 v, float a)  { glColor4f(v.x, v.y, v.z, a); }
void vdbColor(vdbVec4 v)           { glColor4f(v.x, v.y, v.z, v.w); }

void vdbBlendNone()  { glDisable(GL_BLEND); }
void vdbBlendAdd()   { glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE); }
void vdbBlendAlpha() { glEnable(GL_BLEND); glBlendEquation(GL_FUNC_ADD); glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); }

void vdbDepthTest(bool enabled)
{
    if (enabled) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
}
void vdbDepthWrite(bool enabled)
{
    if (enabled) { glDepthMask(GL_TRUE); glDepthRange(0.0f, 1.0f); }
    else { glDepthMask(GL_FALSE); }
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
