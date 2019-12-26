#include "library.h"

extern "C"
{
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

struct scaler_ctx_t
{
    DEFINE_LAST_ERROR

    SwsContext *ctx;
};

LIBRARY_API(scaler_ctx_t *) scaler_context_alloc()
{
    return static_cast<scaler_ctx_t *>(av_mallocz(sizeof(scaler_ctx_t)));
}

LIBRARY_API(void) scaler_context_free(scaler_ctx_t *ctx)
{
    sws_freeContext(ctx->ctx);
    av_free(ctx);
}

LIBRARY_API(int) scaler_scale(AVFrame *out, AVFrame *in, scaler_ctx_t *ctx)
{
    ctx->ctx = sws_getCachedContext(ctx->ctx, in->width, in->height, static_cast<AVPixelFormat>(in->format), out->width,
                                    out->height, static_cast<AVPixelFormat>(out->format), 0, nullptr, nullptr, nullptr);
    return sws_scale(ctx->ctx, in->data, in->linesize, 0, in->height, out->data, out->linesize);
}
