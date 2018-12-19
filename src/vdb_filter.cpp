#include "_super_sample.cpp"
#include "_temporal_blend.cpp"

void vdbEndTAA() { vdb.taa_begun = false; TemporalBlend::End(); }
void vdbBeginTAA(int downscale, float blend_factor)
{
    vdb.taa_begun = true;
    TemporalBlend::Begin(downscale, blend_factor);
}

void vdbEndTSS() { vdb.tss_begun = false; TemporalSuperSample::End(); }
void vdbBeginTSS(int w, int h, int n, float *dx, float *dy)
{
    vdb.tss_begun = true;
    TemporalSuperSample::GetSubpixelOffset(w,h,n,dx,dy);
    TemporalSuperSample::Begin(w, h, n);
}
