#include "_supersample.cpp"
#include "_temporalblend.cpp"

void vdbEndTAA() { TemporalBlend::End(); }
void vdbBeginTAA(int downscale, float blend_factor)
{
    TemporalBlend::Begin(downscale, blend_factor);
}

void vdbEndTSS() { TemporalSuperSample::End(); }
void vdbBeginTSS(int w, int h, int n, float *dx, float *dy)
{
    TemporalSuperSample::GetSubpixelOffset(w,h,n,dx,dy);
    TemporalSuperSample::Begin(w, h, n);
}
