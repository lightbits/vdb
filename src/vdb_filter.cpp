#include "_filter_upsample.cpp"
#include "_filter_lowpass.cpp"

void vdbEndTAA() { vdb.taa_begun = false; lowpass_filter::End(); }
void vdbBeginTAA(int downscale, float blend_factor)
{
    vdb.taa_begun = true;
    lowpass_filter::Begin(downscale, blend_factor);
}

void vdbEndTSS() { vdb.tss_begun = false; upsample_filter::End(); }
void vdbBeginTSS(int w, int h, int n, float *dx, float *dy)
{
    vdb.tss_begun = true;
    upsample_filter::GetSubpixelOffset(w,h,n,dx,dy);
    upsample_filter::Begin(w, h, n);
}
