#include "library.h"
#include <sys/param.h>

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

LIBRARY_API(void) lotus_frame_get_props(int *w, int *h, int *pix_fmt, AVFrame *frame)
{
    *w = frame->width;
    *h = frame->height;
    *pix_fmt = frame->format;
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

LIBRARY_API(int) lotus_packet_len(AVPacket *pkt)
{
    return pkt->size;
}

LIBRARY_API(void) lotus_packet_read(char *buf, int offset, int pos, int count, AVPacket *pkt)
{
    memcpy(buf + offset, pkt->data + pos, count);
}
