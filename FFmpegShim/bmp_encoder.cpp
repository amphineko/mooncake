#include "common.h"
#include "export.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

struct BmpEncoderContext
{
    AVCodecContext *enc;

    int last_error;
    char *last_error_str;
};

LIBRARY_API(void) bmp_context_free(BmpEncoderContext *ctx)
{
    avcodec_free_context(&ctx->enc);
    av_free(ctx);
    av_freep(&ctx->last_error_str);
}

LIBRARY_API(int) bmp_context_open(BmpEncoderContext *ctx, int codec_id, int width, int height, int pix_fmt)
{
    ctx->last_error = 0;
    ctx->last_error_str = static_cast<char *>(av_mallocz(sizeof(char) * MAX_LAST_ERROR_STR));

    auto codec = avcodec_find_encoder(static_cast<AVCodecID>(codec_id));
    ctx->enc = avcodec_alloc_context3(codec);
    ctx->enc->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->enc->height = height;
    ctx->enc->pix_fmt = static_cast<AVPixelFormat>(pix_fmt);
    ctx->enc->time_base.den = ctx->enc->time_base.num = 1;
    ctx->enc->width = width;
    ctx->last_error = avcodec_open2(ctx->enc, codec, nullptr);
    CHECK_ERROR(-1, ctx->last_error != 0, ctx)

    return 0;
}

LIBRARY_API(int) bmp_encode(AVFrame *frame, AVPacket *pkt, BmpEncoderContext *ctx)
{
    ctx->last_error = AVERROR(avcodec_send_frame(ctx->enc, frame));
    CHECK_ERROR(-1, ctx->last_error != 0, ctx)

    ctx->last_error = AVERROR(avcodec_receive_packet(ctx->enc, pkt));
    CHECK_ERROR(-2, ctx->last_error != 0, ctx)

    return 0;
}
