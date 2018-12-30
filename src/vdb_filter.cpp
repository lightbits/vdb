#include "_filter_upsample.cpp"
#include "_filter_lowpass.cpp"

namespace filter
{
    static bool taa_begun;
    static bool tss_begun;
}

void vdbEndTAA() { filter::taa_begun = false; lowpass_filter::End(); }
void vdbBeginTAA(int downscale, float blend_factor)
{
    filter::taa_begun = true;
    lowpass_filter::Begin(downscale, blend_factor);
}

void vdbEndTSS() { filter::tss_begun = false; upsample_filter::End(); }
void vdbBeginTSS(int w, int h, int n, float *dx, float *dy)
{
    filter::tss_begun = true;
    upsample_filter::GetSubpixelOffset(w,h,n,dx,dy);
    upsample_filter::Begin(w, h, n);
}
