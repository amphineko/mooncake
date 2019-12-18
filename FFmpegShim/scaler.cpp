#include "export.h"

extern "C"
{
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

struct ScalerContext
{
    int out_w, out_h, out_fmt;

    SwsContext *ctx;
};

LIBRARY_API(void) scaler_free(ScalerContext *ctx)
{
    sws_freeContext(ctx->ctx);
    ctx->ctx = nullptr;
}

LIBRARY_API(int) scaler_scale(AVFrame *src, AVFrame *out, ScalerContext *ctx)
{
    ctx->ctx = sws_getCachedContext(ctx->ctx, src->width, src->height, static_cast<AVPixelFormat>(src->format),
                                    ctx->out_w, ctx->out_h, static_cast<AVPixelFormat>(ctx->out_fmt), SWS_FAST_BILINEAR,
                                    nullptr, nullptr, nullptr);
    return sws_scale(ctx->ctx, src->data, src->linesize, 0, src->height, out->data, out->linesize);
}
