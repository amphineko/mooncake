#include "export.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
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

LIBRARY_API(AVPacket *) shim_packet_alloc()
{
    return av_packet_alloc();
}

LIBRARY_API(void) shim_packet_free(AVPacket *pkt)
{
    av_packet_free(&pkt);
}

LIBRARY_API(int) shim_packet_len(AVPacket *pkt)
{
    return pkt->size;
}

LIBRARY_API(void) shim_packet_read(char *buf, int buf_offset, int read_offset, int read_count, AVPacket *pkt)
{
    memcpy(buf + buf_offset, pkt->data + read_offset, read_count);
}

LIBRARY_API(void) shim_free(void *ptr)
{
    av_free(ptr);
}
