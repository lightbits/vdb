struct framegrab_options_t
{
    const char *filename; // If no extension is provided, file will be saved as .bmp.
                          // If %d is present, it will be filled with screenshot counter

    bool alpha_channel;
    bool draw_imgui; // If disabled ImGui will not render at all (including overlays)
    bool draw_cursor; // If draw_imgui is false the cursor is a crosshair

    bool reset_counter; // Reset screenshot or image sequence counter
    int start_from; // Set initial screenshot or image sequence counter

    float ffmpeg_fps;
    int ffmpeg_crf; // Quality (lower is better)
    int video_frame_cap; // Stop after capturing this number of frames (0 -> no limit, call StopRecording to stop)
};

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#endif

namespace framegrab
{
    enum framegrab_mode_t { MODE_SCREENSHOT, MODE_SEQUENCE, MODE_FFMPEG };
    static framegrab_options_t options;
    static framegrab_mode_t mode;
    static bool active;
    static int num_frames;
    static int suffix_counter;
    static bool should_stop;

    static void StopRecording()
    {
        should_stop = true;
    }

    static void StartFramegrab(framegrab_options_t _options)
    {
        options = _options;
        if (options.reset_counter)
            suffix_counter = options.start_from;
        num_frames = 0;
        active = true;
        should_stop = false;
    }

    static void TakeScreenshot(framegrab_options_t _options)
    // Save the back framebuffer of the current frame to file.
    {
        mode = MODE_SCREENSHOT;
        StartFramegrab(_options);
    }

    static void RecordImageSequence(framegrab_options_t _options)
    // Save the back framebuffer of the current and each subsequent frame to file.
    {
        mode = MODE_SEQUENCE;
        StartFramegrab(_options);
    }

    static void RecordFFmpeg(framegrab_options_t _options)
    // Pipe the back framebuffer of the current and each subsequent frame to ffmpeg.
    // The function uses _popen to open a pipe, and assumes that the ffmpeg executable is present on the
    // terminal that the application ran from (in the PATH variable). If you're on Windows, you will want
    // to change your PATH environment variable to point to the folder holding the ffmpeg executable.
    {
        mode = MODE_FFMPEG;
        StartFramegrab(_options);
    }

    static void SaveFrame(unsigned char *data,
                          int width,
                          int height,
                          int channels,
                          GLenum format)
    {
        if (mode == MODE_FFMPEG)
        {
            static FILE *ffmpeg = 0;
            if (!ffmpeg)
            {
                // todo: linux/osx
                char cmd[1024];
                sprintf(cmd, "ffmpeg -r %f -f rawvideo -pix_fmt %s -s %dx%d -i - "
                              "-threads 0 -preset fast -y -pix_fmt yuv420p -crf %d -vf vflip %s",
                              options.ffmpeg_fps, // -r
                              options.alpha_channel ? "rgba" : "rgb24", // -pix_fmt
                              width, height, // -s
                              options.ffmpeg_crf, // -crf
                              options.filename);
                ffmpeg = popen(cmd, "wb");
            }

            fwrite(data, width*height*channels, 1, ffmpeg);

            num_frames++;
            if (options.video_frame_cap && num_frames == options.video_frame_cap)
            {
                StopRecording();
            }
            if (should_stop)
            {
                active = false;
                pclose(ffmpeg);
                ffmpeg = 0;
            }
        }
        else
        {
            bool save_as_bmp = false;
            bool save_as_png = false;

            if (strstr(options.filename, ".png"))
            {
                save_as_png = true;
                save_as_bmp = false;
            }
            else if (strstr(options.filename, ".bmp"))
            {
                save_as_bmp = true;
                save_as_png = false;
            }
            else
            {
                save_as_bmp = false;
                save_as_png = false;
                // did user specify any extension at all?
            }

            // todo: what happens if filename doesn't contain a %d?
            char filename[1024];
            sprintf(filename, options.filename, suffix_counter);
            suffix_counter++;

            if (save_as_bmp)
            {
                stbi_write_bmp(filename, width, height, channels, data);
                printf("Saved %s...\n", filename);
            }
            else if (save_as_png)
            {
                int stride = width*channels;
                stbi_write_png(filename, width, height, channels, data+stride*(height-1), -stride);
                printf("Saved %s...\n", filename);
            }
            else
            {
                stbi_write_bmp(filename, width, height, channels, data);
                printf("Saved %s (bmp)...\n", filename);
            }

            if (mode == MODE_SEQUENCE)
            {
                num_frames++;
                if (options.video_frame_cap > 0 && num_frames == options.video_frame_cap)
                {
                    StopRecording();
                }
                if (should_stop)
                {
                    active = false;
                }
            }
            else
            {
                active = false;
            }
        }
    }
}
