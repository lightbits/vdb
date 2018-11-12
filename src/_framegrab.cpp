#pragma once

static void StopFramegrab();

static void TakeScreenshot(const char *filename, bool imgui=false, bool cursor=false, bool reset=false, bool alpha=false);
// Save the framebuffer at the next main render loop and save it to a .png or .bmp.
// imgui: Enable drawing GUI
// cursor: Enable drawing a cursor or not (If drawing GUI, the cursor is arrow cursor otherwise you get a crosshair)
// reset: Reset screenshot frame counter
// alpha: Extract alpha channel from framebuffer
// filename: If no extension is provided, file will be saved as .bmp.
//           If %d is present, it will be filled with screenshot counter

static void RecordVideoToImageSequence(const char *filename, int frame_cap, bool imgui=false, bool cursor=false, bool reset=false, bool alpha=false);
// Save the framebuffer starting from the next main render loop and save it to a sequence of .png or .bmp.
// frame_cap: Stop after capturing this number of frames (0 for no limit, call StopFramegrab to stop manually)
// imgui: Enable drawing GUI
// cursor: Enable drawing a cursor or not (If drawing GUI, the cursor is arrow cursor otherwise you get a crosshair)
// reset: Reset screenshot frame counter
// alpha: Extract alpha channel from framebuffer
// filename: If no extension is provided, file will be saved as .bmp.
//           If %d is present, it will be filled with screenshot counter

static void RecordVideoToFfmpeg(const char *filename, float fps, int crf, int frame_cap, bool imgui=false, bool cursor=false, bool alpha=false);
// Get the framebuffer starting from the next main render loop and pipe them directly to ffmpeg
// fps: Framerate
// crf: Quality (lower is better)
// frame_cap: Stop after capturing this number of frames (0 for no limit, call StopFramegrab to stop manually)
// imgui: Enable drawing GUI
// cursor: Enable drawing a cursor or not (If drawing GUI, the cursor is arrow cursor otherwise you get a crosshair)
// reset: Reset video frame counter
// alpha: Extract alpha channel from framebuffer
// filename: If no extension is provided, file will be saved as .bmp.
//           If %d is present, it will be filled with video frame counter
//
// The function uses _popen to open a pipe, and assumes that the ffmpeg executable is present on the
// terminal that the application ran from (in the PATH variable). If you're on Windows, you will want
// to change your PATH environment variable to point to the folder holding the ffmpeg executable.

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#else
#endif

struct framegrab_options_t
{
    const char *filename;
    bool alpha_channel;
    bool draw_cursor;
    bool draw_imgui;
    bool is_video;
    bool use_ffmpeg;
    float ffmpeg_fps;
    int ffmpeg_crf;
    bool reset_num_screenshots;
    bool reset_num_video_frames;
    int start_from;
    int video_frame_cap;
};

struct framegrab_t
{
    framegrab_options_t options;
    bool active;
    int num_screenshots;
    int num_video_frames;
    bool should_stop;
};
static framegrab_t framegrab = {0};

static void StopFramegrab()
{
    framegrab.should_stop = true;
}

static void StartFrameGrab(framegrab_options_t opt)
{
    framegrab.options = opt;
    if (opt.reset_num_screenshots)
        framegrab.num_screenshots = opt.start_from;
    if (opt.reset_num_video_frames)
        framegrab.num_video_frames = opt.start_from;
    framegrab.active = true;
    framegrab.should_stop = false;
}

static void TakeScreenshot(
    const char *filename, bool imgui, bool cursor, bool reset, int start_from, bool alpha)
{
    framegrab_options_t opt = {0};
    opt.filename = filename;
    opt.reset_num_screenshots = reset;
    opt.start_from = start_from;
    opt.draw_imgui = imgui;
    opt.draw_cursor = cursor;
    opt.alpha_channel = alpha;
    StartFrameGrab(opt);
}

static void RecordVideoToImageSequence(
    const char *filename, int frame_cap, bool imgui, bool cursor, bool reset, int start_from, bool alpha)
{
    framegrab_options_t opt = {0};
    opt.filename = filename;
    opt.reset_num_video_frames = reset;
    opt.start_from = start_from;
    opt.draw_imgui = imgui;
    opt.draw_cursor = cursor;
    opt.alpha_channel = alpha;
    opt.is_video = true;
    opt.video_frame_cap = frame_cap;
    StartFrameGrab(opt);
}

static void RecordVideoToFfmpeg(
    const char *filename, float fps, int crf, int frame_cap, bool imgui, bool cursor, bool alpha)
{
    framegrab_options_t opt = {0};
    opt.filename = filename;
    opt.draw_imgui = imgui;
    opt.draw_cursor = cursor;
    opt.alpha_channel = alpha;
    opt.is_video = true;
    opt.use_ffmpeg = true;
    opt.ffmpeg_fps = fps;
    opt.ffmpeg_crf = crf;
    opt.video_frame_cap = frame_cap;
    StartFrameGrab(opt);
}

static void FramegrabSaveOutput(unsigned char *data, int width, int height, int channels, GLenum format)
{
    framegrab_options_t opt = framegrab.options;

    // write output to ffmpeg or to file
    if (opt.use_ffmpeg)
    {
        static FILE *ffmpeg = 0;
        if (!ffmpeg)
        {
            // todo: linux/osx
            char cmd[1024];
            sprintf(cmd, "ffmpeg -r %f -f rawvideo -pix_fmt %s -s %dx%d -i - "
                          "-threads 0 -preset fast -y -pix_fmt yuv420p -crf %d -vf vflip %s",
                          opt.ffmpeg_fps, // -r
                          opt.alpha_channel ? "rgba" : "rgb24", // -pix_fmt
                          width, height, // -s
                          opt.ffmpeg_crf, // -crf
                          opt.filename);
            ffmpeg = popen(cmd, "wb");
        }

        fwrite(data, width*height*channels, 1, ffmpeg);

        framegrab.num_video_frames++;
        if (opt.video_frame_cap && framegrab.num_video_frames == opt.video_frame_cap)
        {
            StopFramegrab();
        }
        if (framegrab.should_stop)
        {
            framegrab.active = false;
            pclose(ffmpeg);
            ffmpeg = 0;
        }
    }
    else
    {
        bool save_as_bmp = false;
        bool save_as_png = false;

        if (strstr(opt.filename, ".png"))
        {
            save_as_png = true;
            save_as_bmp = false;
        }
        else if (strstr(opt.filename, ".bmp"))
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

        char filename[1024];
        if (opt.is_video)
        {
            // todo: check if filename template has %...d?
            sprintf(filename, opt.filename, framegrab.num_video_frames);
        }
        else
        {
            // todo: check if filename template has %...d?
            sprintf(filename, opt.filename, framegrab.num_screenshots);
        }

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

        if (opt.is_video)
        {
            framegrab.num_video_frames++;
            if (opt.video_frame_cap && framegrab.num_video_frames == opt.video_frame_cap)
            {
                StopFramegrab();
            }
            if (framegrab.should_stop)
            {
                framegrab.active = false;
            }
        }
        else
        {
            framegrab.num_screenshots++;
            framegrab.active = false;
        }
    }
}
