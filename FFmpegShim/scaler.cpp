#include "export.h"

extern "C"
{
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

struct ScalerContext
{
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
                                    out->width, out->height, static_cast<AVPixelFormat>(out->format), SWS_FAST_BILINEAR,
                                    nullptr, nullptr, nullptr);
    return sws_scale(ctx->ctx, src->data, src->linesize, 0, src->height, out->data, out->linesize);
}
