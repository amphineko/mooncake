#include "export.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
}

struct RtspClientContext
{
    AVFormatContext *fmt;

    AVStream *stream;

    AVCodec *dec;
    AVCodecContext *dec_ctx;

    int last_error;
};

LIBRARY_API(void) rtsp_close(RtspClientContext *ctx)
{
    if (ctx->dec_ctx)
    {
        avcodec_close(ctx->dec_ctx);
        avcodec_free_context(&ctx->dec_ctx);
    }

    if (ctx->fmt)
        avformat_close_input(&ctx->fmt);
}

LIBRARY_API(const char *) rtsp_get_codec_name(RtspClientContext *ctx)
{
    return ctx->dec->name;
}

LIBRARY_API(const char *) rtsp_get_pixel_format_name(RtspClientContext *ctx)
{
    return av_get_pix_fmt_name(ctx->dec_ctx->pix_fmt);
}

LIBRARY_API(int) rtsp_open(char *uri, RtspClientContext *ctx)
{
    avformat_network_init();

    // open rtsp transport
    ctx->last_error = AVERROR(avformat_open_input(&ctx->fmt, uri, nullptr, nullptr));
    if (ctx->last_error)
    {
        fprintf(stderr, "[rtsp_open] avformat_open_input failed: %d\n", ctx->last_error);
        return -1;
    }

    // retrieve stream information
    auto find_stream_ret = avformat_find_stream_info(ctx->fmt, nullptr);
    if (find_stream_ret < 0)
    {
        ctx->last_error = AVERROR(find_stream_ret);
        fprintf(stderr, "[rtsp_open] avformat_find_stream_info failed: %d\n", ctx->last_error);
        return -2;
    }

    // open default video stream
    auto stream_idx = av_find_best_stream(ctx->fmt, AVMEDIA_TYPE_VIDEO, -1, -1, &ctx->dec, 0);
    if (stream_idx < 0)
    {
        fprintf(stderr, "[rtsp_open] no candidate video stream\n");
        return -3;
    }

    // find stream codec
    auto stream = ctx->fmt->streams[stream_idx];
    ctx->dec_ctx = avcodec_alloc_context3(ctx->dec);

    // open stream decoder
    AVDictionary *dec_opts = nullptr;
    av_dict_set(&dec_opts, "refcounted_frames", "1", 0);
    auto open_ret = avcodec_open2(ctx->dec_ctx, ctx->dec, &dec_opts);
    if (open_ret < 0)
    {
        fprintf(stderr, "[rtsp_open] failed to open codec %s: %d", ctx->dec->name, open_ret);
        return -5;
    }

    return 0;
}

LIBRARY_API(int) rtsp_receive_frame(AVFrame *frame, RtspClientContext *ctx)
{
    return AVERROR(avcodec_receive_frame(ctx->dec_ctx, frame));
}
