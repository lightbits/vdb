namespace lowpass_filter
{
    static render_texture_t rt_frame = {0};
    static render_texture_t rt_accumulator[2] = {0};
    static int turn = 0;
    static float blend_factor;
    static bool has_begun;

    void Begin(int downsample, float _blend_factor)
    {
        using namespace render_texture;

        has_begun = true;
        blend_factor = _blend_factor;
        int w = window::framebuffer_width;
        int h = window::framebuffer_height;
        if (rt_frame.width != w>>downsample || rt_frame.height != h>>downsample)
        {
            FreeRenderTexture(&rt_frame);
            FreeRenderTexture(&rt_accumulator[0]);
            FreeRenderTexture(&rt_accumulator[1]);
            rt_frame = MakeRenderTexture(w>>downsample, h>>downsample, GL_NEAREST, GL_NEAREST, true);
            rt_accumulator[0] = MakeRenderTexture(w, h, GL_NEAREST, GL_NEAREST, false, GL_RGBA, GL_FLOAT, GL_RGBA32F);
            rt_accumulator[1] = MakeRenderTexture(w, h, GL_NEAREST, GL_NEAREST, false, GL_RGBA, GL_FLOAT, GL_RGBA32F);

            for (int i = 0; i < 2; i++)
            {
                EnableRenderTexture(&rt_accumulator[i]);
                glClearColor(0,0,0,1);
                glClear(GL_COLOR_BUFFER_BIT);
                DisableRenderTexture(&rt_accumulator[i]);
            }
        }
        EnableRenderTexture(&rt_frame);
    }
    void End()
    {
        using namespace render_texture;

        static GLuint program = 0;
        static GLint attrib_position = 0;
        static GLint uniform_factor = 0;
        static GLint uniform_sampler0 = 0;
        static GLint uniform_sampler1 = 0;
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
            uniform sampler2D sampler0; // current frame
            uniform sampler2D sampler1; // last accumulator
            uniform float factor;
            out vec4 out_color; // current accumulator
            void main()
            {
                // out_color = texture(sampler0,texel);
                vec4 current_frame = texture(sampler0,texel);
                vec4 last_accumulator = texture(sampler1,texel);
                vec4 current_accumulator = factor*last_accumulator + (1.0-factor)*current_frame;
                out_color = current_accumulator;

                // todo: alphablending, alpha buffers are kinda broken with this?
            }
            );
            #undef SHADER

            program = LoadShaderFromMemory(vs,fs);
            attrib_position = glGetAttribLocation(program, "position");
            uniform_sampler0 = glGetUniformLocation(program, "sampler0");
            uniform_sampler1 = glGetUniformLocation(program, "sampler1");
            uniform_factor = glGetUniformLocation(program, "factor");
            assert(attrib_position >= 0 && "Unused or nonexistent attribute");
            assert(uniform_sampler0 >= 0 && "Unused or nonexistent uniform");
            assert(uniform_sampler1 >= 0 && "Unused or nonexistent uniform");
            assert(uniform_factor >= 0 && "Unused or nonexistent uniform");
            assert(program && "Failed to compile TemporalBlend shader");
        }

        has_begun = false;

        int next_turn = (turn+1)%2;
        int back = turn;
        int front = next_turn;

        DisableRenderTexture(&rt_frame);
        StoreGLState();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);

        // add just-rendered frame to accumulator framebuffer
        {
            static const float position[] = { -1,-1, 1,-1, 1,1, 1,1, -1,1, -1,-1 };

            EnableRenderTexture(&rt_accumulator[front]);
            glUseProgram(program);
            glActiveTexture(GL_TEXTURE1);
            glUniform1i(uniform_sampler1, 1);
            glBindTexture(GL_TEXTURE_2D, rt_accumulator[back].color[0]);
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(uniform_sampler0, 0);
            glBindTexture(GL_TEXTURE_2D, rt_frame.color[0]);
            glUniform1f(uniform_factor, blend_factor);
            glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, position);
            glEnableVertexAttribArray(attrib_position);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisableVertexAttribArray(attrib_position);
            glUseProgram(0);
            DisableRenderTexture(&rt_accumulator[front]);
        }

        // draw blended result to default framebuffer
        {
            glBindTexture(GL_TEXTURE_2D, rt_accumulator[front].color[0]);
            glBegin(GL_TRIANGLES);
            glColor4f(1,1,1,1);
            glTexCoord2f(0,0);glVertex2f(-1,-1);
            glTexCoord2f(1,0);glVertex2f(+1,-1);
            glTexCoord2f(1,1);glVertex2f(+1,+1);
            glTexCoord2f(1,1);glVertex2f(+1,+1);
            glTexCoord2f(0,1);glVertex2f(-1,+1);
            glTexCoord2f(0,0);glVertex2f(-1,-1);
            glEnd();
        }

        RestoreGLState();

        turn = next_turn;
    }
}

