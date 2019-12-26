#include "library.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

struct frame_encoder_ctx_t
{
    DEFINE_LAST_ERROR

    AVCodecContext *enc;
};

LIBRARY_API(frame_encoder_ctx_t *) encoder_context_create()
{
    return static_cast<frame_encoder_ctx_t *>(av_mallocz(sizeof(frame_encoder_ctx_t)));
}

LIBRARY_API(void) encoder_context_free(frame_encoder_ctx_t *ctx)
{
    av_freep(&ctx->enc);
    av_free(ctx);
}

LIBRARY_API(int) encoder_context_open(int codec_id, int w, int h, int pix_fmt, frame_encoder_ctx_t *ctx)
{
    auto codec = avcodec_find_encoder(static_cast<AVCodecID>(codec_id));
    CHECK_LAV_ERROR(-1, codec == nullptr, "avcodec_find_encoder", ctx)

    ctx->enc = avcodec_alloc_context3(codec);
    ctx->enc->width = w;
    ctx->enc->height = h;
    ctx->enc->pix_fmt = static_cast<AVPixelFormat>(pix_fmt);
    ctx->enc->time_base = {1, 1};
    ctx->last_error = avcodec_open2(ctx->enc, codec, nullptr);
    CHECK_LAV_ERROR_NON_ZERO(-2, "avcodec_open2", ctx)

    return 0;
}

LIBRARY_API(int) encoder_encode(AVPacket *out, AVFrame *in, frame_encoder_ctx_t *ctx)
{
    auto enc = ctx->enc;

    ctx->last_error = avcodec_send_frame(enc, in);
    CHECK_LAV_ERROR_NON_ZERO(-1, "avcodec_send_frame", ctx)

    ctx->last_error = avcodec_receive_packet(enc, out);
    CHECK_LAV_ERROR_NON_ZERO(-2, "avcodec_receive_packet", ctx)

    return 0;
}
