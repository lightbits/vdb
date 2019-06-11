typedef framebuffer_t render_target_t;

enum { MAX_RENDER_TARGETS = 1024 };
static framebuffer_t render_targets[MAX_RENDER_TARGETS];

void vdbBeginRenderTarget(int slot, vdbRenderTargetSize size, vdbRenderTargetFormat format)
{
    assert(slot >= 0 && slot < MAX_RENDER_TARGETS && "You are trying to use a render texture beyond the available slots.");
    assert(format.stencil_bits == 0 && "Stencil in RenderTarget is not implemented yet.");

    render_target_t *rt = render_targets + slot;
    bool should_create = false;
    if (rt->fbo)
    {
        if (rt->width != size.width || rt->height != size.height)
        {
            FreeFramebuffer(rt);
            should_create = true;
        }
    }
    else
    {
        should_create = true;
    }

    if (should_create)
    {
        GLenum data_format = GL_RGBA;
        GLenum data_type = GL_UNSIGNED_BYTE;
        GLenum internal_format = TextureFormatToGL(format.format);
        bool enable_depth = (format.depth_bits > 0);

        *rt = MakeFramebuffer(size.width, size.height, GL_LINEAR, GL_LINEAR, enable_depth, data_format, data_type, internal_format);
        EnableFramebuffer(rt);
        glClearColor(0,0,0,0);
        if (enable_depth) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        else glClear(GL_COLOR_BUFFER_BIT);
        DisableFramebuffer(rt);
    }

    EnableFramebuffer(rt);
}

void vdbEndRenderTarget(int slot)
{
    assert(slot >= 0 && slot < MAX_RENDER_TARGETS && "You are trying to use a render texture beyond the available slots.");
    DisableFramebuffer(render_targets + slot);

    // todo: if LINEAR_MIPMAP, regenerate mipmaps
}

void vdbBindRenderTarget(int slot, vdbTextureFilter filter, vdbTextureWrap wrap)
{
    assert(slot >= 0 && slot < MAX_RENDER_TARGETS && "You are trying to use a render texture beyond the available slots.");
    glActiveTexture(GL_TEXTURE0); // todo: let user specify this
    glBindTexture(GL_TEXTURE_2D, render_targets[slot].color[0]);
    vdbSetTextureParameters(filter, wrap);
}

void vdbUnbindRenderTarget()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawRenderTargetWithDepth(render_target_t rt)
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
        out vec4 out_color;
        void main()
        {
            out_color = texture(sampler0, texel);
            gl_FragDepth = texture(sampler1, texel).x;
        }
    );
    #undef SHADER

    static GLuint program = LoadShaderFromMemory(vs,fs);
    static GLint attrib_position = glGetAttribLocation(program, "position");
    static GLint uniform_sampler0 = glGetUniformLocation(program, "sampler0");
    static GLint uniform_sampler1 = glGetUniformLocation(program, "sampler1");

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

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glUseProgram(program);
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(uniform_sampler1, 1);
    glBindTexture(GL_TEXTURE_2D, rt.depth);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uniform_sampler0, 0);
    glBindTexture(GL_TEXTURE_2D, rt.color[0]);
    glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(attrib_position);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(attrib_position);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void vdbDrawRenderTarget(int slot, vdbTextureFilter filter, vdbTextureWrap wrap)
{
    assert(slot >= 0 && slot < MAX_RENDER_TARGETS && "You are trying to use a render texture beyond the available slots.");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_targets[slot].color[0]);
    vdbSetTextureParameters(filter, wrap);
    vdbBeginTriangles();
    vdbColor(1,1,1,1);
    vdbTexel(0,0); vdbVertex(-1,-1);
    vdbTexel(1,0); vdbVertex(+1,-1);
    vdbTexel(1,1); vdbVertex(+1,+1);
    vdbTexel(1,1); vdbVertex(+1,+1);
    vdbTexel(0,1); vdbVertex(-1,+1);
    vdbTexel(0,0); vdbVertex(-1,-1);
    vdbEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}
