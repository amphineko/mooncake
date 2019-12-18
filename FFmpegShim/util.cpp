#include "export.h"

extern "C"
{
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
}

LIBRARY_API(AVFrame *) shim_frame_alloc()
{
    return av_frame_alloc();
}

LIBRARY_API(int) shim_frame_alloc_buffer(int w, int h, int pix_fmt, AVFrame *frame)
{
    frame->format = pix_fmt;
    frame->width = w;
    frame->height = h;
    return AVERROR(av_frame_get_buffer(frame, 32));
}

LIBRARY_API(void) shim_frame_free(AVFrame *frame)
{
    av_frame_free(&frame);
}

LIBRARY_API(void) shim_free(void *ptr)
{
    av_free(ptr);
}
