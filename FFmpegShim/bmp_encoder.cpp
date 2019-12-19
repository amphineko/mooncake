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

LIBRARY_API(int) bmp_context_open(BmpEncoderContext *ctx, int codec_id)
{
    auto codec = avcodec_find_encoder(codec_id);
    return avcodec_open2(ctx->ctx, codec, nullptr);
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
