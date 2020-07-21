// The temporal super sampler renders a high-resolution framebuffer
// with dimensions (W<<N,H<<N) where N is the upsampling factor.
// This framebuffer is linearly downsampled and blitted to the window.
// The rendering is done over several frames where each frame is assigned
// to a regular sampling of the high-resolution buffer. Here for example
// is a 3x2 framebuffer supersampled to 6x4.
//                          -----------------
//                         | 2 3 | 2 3 | 2 3 |
//                         | 0 1 | 0 1 | 0 1 |
//                         |-----+-----+-----|
//                         | 2 3 | 2 3 | 2 3 |
//                         | 0 1 | 0 1 | 0 1 |
//                          -----------------
// Each number refers to the frame at which that pixel gets rendered.
// For example, in frame 0, only the upper-left pixels are rendered
// to a low-resolution framebuffer (here 3x2), and then distributed
// to the high-res framebuffer:
//                                   _________________
//                 ___________      | # # | # # | # # |
//                | 0 | 0 | 0 |     | 0 # | 0 # | 0 # |
//                | --+---+---| --> |-----+-----+-----|
//                | 0 | 0 | 0 |     | # # | # # | # # |
//                 -----------      | 0 # | 0 # | 0 # |
//                                   -----------------
// In this example, it would take 4 frames before the output settles,
// assuming a static scene.
namespace render_scaler
{
    static framebuffer_t output;
    static framebuffer_t lowres;
    static int subpixel;
    static bool has_begun;
    static int scale_up;
    static int sample_pos_idx;
    static int sample_pos_idy;
    static float frag_offset_x;
    static float frag_offset_y;
    static float sample_pos_ndc_x;
    static float sample_pos_ndc_y;

    void Begin(int w, int h, int n_up)
    {
        window::SetMinimumNumSettleFrames(3 + (1<<n_up)*(1<<n_up));

        scale_up = n_up;
        has_begun = true;

        if (lowres.width != w || lowres.height != h)
        {
            FreeFramebuffer(&lowres);
            lowres = MakeFramebuffer(w, h, GL_NEAREST, GL_NEAREST, true);
            subpixel = 0;
        }
        if (output.width != w<<n_up || output.height != h<<n_up)
        {
            FreeFramebuffer(&output);
            output = MakeFramebuffer(w<<n_up, h<<n_up, GL_NEAREST, GL_NEAREST, true);
            subpixel = 0;
            EnableFramebuffer(&output);
            // Although we do _eventually_ overwrite all pixels in
            // the output RT, the user may see some garbage frames
            // after a new RT is created, unless it's cleared.
            GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
            GLboolean depth_mask; glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
            vdbDepthWrite(true);
            vdbDepthTest(true);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            vdbDepthWrite(depth_mask);
            vdbDepthTest(depth_test);
            DisableFramebuffer(&output);
        }

        EnableFramebuffer(&lowres);

        // We don't assume the user will clear the RT after calling
        // BeginRenderScaler, as it's easy to forget it. So we clear
        // it for them.
        {
            GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
            GLboolean depth_mask; glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
            vdbDepthWrite(true);
            vdbDepthTest(true);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            vdbDepthWrite(depth_mask);
            vdbDepthTest(depth_test);
        }

        // calculate the sample position for this frame
        {
            // todo: nicer sampling pattern for any upsampling factor
            int i = subpixel;
            if (scale_up == 2)
            {
                static const int remap[] = {
                    10, 5, 16, 14,
                    12, 2,  1, 11,
                     3, 7,  9, 15,
                     4, 8, 13,  6
                };
                i = remap[subpixel]-1;
            }
            else if (scale_up == 3)
            {
                static const int remap[] = {42, 3, 8, 34, 18, 38, 25, 61, 16, 48, 56, 7, 55, 32, 59, 41, 4, 36, 30, 1, 24, 50, 0, 12, 39, 45, 31, 10, 58, 14, 23, 43, 46, 44, 9, 40, 28, 6, 17, 15, 60, 27, 37, 52, 20, 53, 19, 62, 47, 33, 63, 13, 5, 49, 54, 26, 35, 22, 21, 57, 29, 2, 51, 11};
                i = remap[subpixel];
            }
            sample_pos_idx = i % (1<<scale_up);
            sample_pos_idy = i / (1<<scale_up);

            int num_samples = 1<<scale_up;
            frag_offset_x = (sample_pos_idx + 0.5f)/(float)num_samples - 0.5f;
            frag_offset_y = (sample_pos_idy + 0.5f)/(float)num_samples - 0.5f;

            float pixel_width_ndc = 2.0f / (w<<scale_up);
            float pixel_height_ndc = 2.0f / (h<<scale_up);
            sample_pos_ndc_x = (-0.5f*num_samples + 0.5f + sample_pos_idx)*pixel_width_ndc;
            sample_pos_ndc_y = (-0.5f*num_samples + 0.5f + sample_pos_idy)*pixel_height_ndc;
        }
    }
    void End()
    {
        static GLuint program = 0;
        static GLint attrib_position = 0;
        static GLint uniform_sampler0 = 0;
        static GLint uniform_sampler1 = 0;
        static GLint uniform_dx = 0;
        static GLint uniform_dy = 0;
        static GLint uniform_nx = 0;
        if (!program)
        {
            #define SHADER(S) "#version 150\n" #S
            const char *vs = SHADER(
            in vec2 position;
            out vec2 texel;
            void main()
            {
                texel = vec2(0.5) + 0.5*position;
                gl_Position = vec4(position, 0.0, 1.0);
            }
            );

            const char *fs = SHADER(
            in vec2 texel;
            uniform sampler2D sampler0;
            uniform sampler2D sampler1;
            uniform float dx;
            uniform float dy;
            uniform float nx;
            out vec4 out_color;
            void main()
            {
                float ix = trunc(mod(gl_FragCoord.x,nx));
                float iy = trunc(mod(gl_FragCoord.y,nx));
                if (ix == dx && iy == dy)
                {
                    out_color = texture(sampler0, texel);
                    gl_FragDepth = texture(sampler1, texel).x;
                }
                else
                {
                    discard;
                }
            }
            );
            #undef SHADER

            program = LoadShaderFromMemory(vs,fs);
            assert(program && "Failed to compile TemporalBlend shader");
            attrib_position = glGetAttribLocation(program, "position");
            uniform_sampler0 = glGetUniformLocation(program, "sampler0");
            uniform_sampler1 = glGetUniformLocation(program, "sampler1");
            uniform_dx = glGetUniformLocation(program, "dx");
            uniform_dy = glGetUniformLocation(program, "dy");
            uniform_nx = glGetUniformLocation(program, "nx");
            assert(attrib_position >= 0 && "Unused or nonexistent attribute");
            assert(uniform_sampler0 >= 0 && "Unused or nonexistent uniform");
            assert(uniform_dx >= 0 && "Unused or nonexistent uniform");
            assert(uniform_dy >= 0 && "Unused or nonexistent uniform");
            assert(uniform_nx >= 0 && "Unused or nonexistent uniform");
        }

        has_begun = false;
        DisableFramebuffer(&lowres);

        imm_state_t last_state = immediate::GetState();
        vdbPushProjection();
        vdbPushMatrix();
        vdbLoadProjection(NULL);
        vdbLoadMatrix(NULL);
        vdbBlendNone();
        vdbCullFace(false);
        vdbDepthWrite(true);
        vdbDepthTest(true); // When depth testing is disabled, writes to the depth buffer are also disabled.
        vdbDepthFuncAlways();
        immediate::SetRenderOffsetNDC(vdbVec2(0.0f, 0.0f));

        // interleave just-rendered frame into full resolution framebuffer
        {
            static GLuint vao = 0;
            static GLuint vbo = 0;
            if (!vao)
            {
                static float position[] = { -1,-1, 1,-1, 1,1, 1,1, -1,1, -1,-1 };
                glGenVertexArrays(1, &vao);
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
            }
            assert(vao);
            assert(vbo);

            EnableFramebuffer(&output);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glUseProgram(program);
            glActiveTexture(GL_TEXTURE1);
            glUniform1i(uniform_sampler1, 1);
            glBindTexture(GL_TEXTURE_2D, lowres.depth);
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(uniform_sampler0, 0);
            glBindTexture(GL_TEXTURE_2D, lowres.color[0]);
            glUniform1f(uniform_dx, (float)sample_pos_idx);
            glUniform1f(uniform_dy, (float)sample_pos_idy);
            glUniform1f(uniform_nx, (float)(1<<scale_up));
            glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(attrib_position);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisableVertexAttribArray(attrib_position);
            glUseProgram(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            DisableFramebuffer(&output);
        }

        // composite full resolution framebuffer onto window framebuffer
        // note: we re-enable user's draw state (e.g. depth test/write).
        immediate::SetState(last_state);
        DrawRenderTargetWithDepth(output, VDB_NEAREST, VDB_CLAMP);

        vdbPopMatrix();
        vdbPopProjection();

        // just a linear sampling order. want something nicer in the future
        int num_subpixels = (1<<scale_up)*(1<<scale_up);
        subpixel = (subpixel+1)%num_subpixels;
    }
}

// You can apply this value to a perspective projection matrix'
// (0,2) and (1,2) elements:
// . . dx 0
// . . dy 0
// . .  . .
// . .  . 1
// such that, after dividing by w (=-z), you get a z-independent
// shift of all fragments.
vdbVec2 vdbGetRenderOffset()
{
    using namespace render_scaler;
    if (has_begun)
        return vdbVec2(sample_pos_ndc_x, sample_pos_ndc_y);
    else
        return vdbVec2(0.0f, 0.0f);
}

vdbVec2 vdbGetRenderOffsetFramebuffer()
{
    using namespace render_scaler;
    if (has_begun)
        return vdbVec2(frag_offset_x, frag_offset_y);
    else
        return vdbVec2(0.0f, 0.0f);
}

vdbVec2 vdbGetRenderScale()
{
    return vdbVec2((float)vdbGetFramebufferWidth()/vdbGetWindowWidth(),
                   (float)vdbGetFramebufferHeight()/vdbGetWindowHeight());
}

void vdbBeginRenderScale(int width, int height, int up)
{
    assert(!render_scaler::has_begun && "You have to disable the built-in render scaler (set to 1/1 in settings).");
    assert(up >= 0);
    render_scaler::Begin(width, height, up);
    immediate::SetRenderOffsetNDC(vdbGetRenderOffset());
}

void vdbBeginRenderScale(int down, int up)
{
    assert(!render_scaler::has_begun && "You have to disable the built-in render scaler (set to 1/1 in settings).");
    assert(down >= 0);
    assert(up >= 0);
    int width = window::framebuffer_width >> down;
    int height = window::framebuffer_height >> down;
    vdbBeginRenderScale(width, height, up);
}

void vdbEndRenderScale()
{
    assert(render_scaler::has_begun);
    render_scaler::End();
    immediate::SetRenderOffsetNDC(vdbGetRenderOffset());
}
