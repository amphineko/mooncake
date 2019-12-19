#include "export.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

struct BmpEncoderContext
{
    AVCodecContext *enc;

    int last_error;
};

LIBRARY_API(void) bmp_context_free(BmpEncoderContext *ctx)
{
    avcodec_free_context(&ctx->enc);
    av_free(ctx);
}

LIBRARY_API(int) bmp_context_open(BmpEncoderContext *ctx, int codec_id, int w, int h, int pix_fmt)
{
    auto codec = avcodec_find_encoder(static_cast<AVCodecID>(codec_id));
    ctx->enc = avcodec_alloc_context3(codec);
    ctx->enc->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->enc->height = h;
    ctx->enc->pix_fmt = static_cast<AVPixelFormat>(pix_fmt);
    ctx->enc->time_base.den = ctx->enc->time_base.num = 1;
    ctx->enc->width = w;
    return avcodec_open2(ctx->enc, codec, nullptr);
}

LIBRARY_API(int) bmp_encode(AVFrame *frame, AVPacket *pkt, BmpEncoderContext *ctx)
{
    ctx->last_error = AVERROR(avcodec_send_frame(ctx->enc, frame));
    if (ctx->last_error != 0)
        return -1;

    ctx->last_error = AVERROR(avcodec_receive_packet(ctx->enc, pkt));
    if (ctx->last_error != 0)
        return -2;

    return 0;
}
