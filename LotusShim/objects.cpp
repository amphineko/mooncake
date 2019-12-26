#include "library.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
}

LIBRARY_API(AVFrame *) lotus_frame_alloc()
{
    return av_frame_alloc();
}

LIBRARY_API(void) lotus_frame_free(AVFrame *frame)
{
    av_frame_free(&frame);
}

LIBRARY_API(void) lotus_frame_init(int w, int h, int pix_fmt, AVFrame *frame)
{
    frame->width = w;
    frame->height = h;
    frame->format = pix_fmt;
    av_frame_get_buffer(frame, 32);
}

LIBRARY_API(AVPacket *) lotus_packet_alloc()
{
    return av_packet_alloc();
}

LIBRARY_API(void) lotus_packet_free(AVPacket *pkt)
{
    av_packet_free(&pkt);
}
