#include "export.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
}

struct RtspClientContext
{
    AVFormatContext *fmt;

    AVCodec *codec;
    AVCodecContext *dec;
    int video_idx;

    AVPacket *pkt;

    int last_error;
};

LIBRARY_API(void) rtsp_free(RtspClientContext *ctx)
{
    av_read_pause(ctx->fmt);

    if (ctx->dec)
    {
        avcodec_close(ctx->dec);
        avcodec_free_context(&ctx->dec);
    }

    if (ctx->fmt)
        avformat_close_input(&ctx->fmt);
}

LIBRARY_API(const char *) rtsp_get_codec_name(RtspClientContext *ctx)
{
    return ctx->codec->name;
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

    // find default video stream
    auto stream_idx = av_find_best_stream(ctx->fmt, AVMEDIA_TYPE_VIDEO, -1, -1, &ctx->codec, 0);
    if (stream_idx < 0)
    {
        fprintf(stderr, "[rtsp_open] no candidate video stream\n");
        return -3;
    }
    ctx->video_idx = stream_idx;

    // create decoder
    auto stream = ctx->fmt->streams[stream_idx];
    ctx->dec = avcodec_alloc_context3(ctx->codec);

    // open stream decoder
    AVDictionary *dec_opts = nullptr;
    av_dict_set(&dec_opts, "refcounted_frames", "1", 0);
    auto open_ret = avcodec_open2(ctx->dec, ctx->codec, &dec_opts);
    if (open_ret < 0)
    {
        fprintf(stderr, "[rtsp_open] failed to open codec %s: %d", ctx->codec->name, open_ret);
        return -5;
    }

    // allocate packet for receiving
    ctx->pkt = av_packet_alloc();

    // start playback (?)
    av_read_play(ctx->fmt);

    return 0;
}

LIBRARY_API(int) rtsp_receive_frame(AVFrame *frame, RtspClientContext *ctx)
{
    ctx->last_error = av_read_frame(ctx->fmt, ctx->pkt);
    if (ctx->last_error)
        return -1; // read frame packet failed
    if (ctx->pkt->stream_index != ctx->video_idx)
        return EAGAIN; // not video frame

    ctx->last_error = avcodec_send_packet(ctx->dec, ctx->pkt);
    if (ctx->last_error)
        return -2; // queue decoding failed

    ctx->last_error = avcodec_receive_frame(ctx->dec, frame);
    if (ctx->last_error)
        return -3; // retrieve decoding failed

    return 0;
}
