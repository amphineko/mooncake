#include "common.h"
#include "export.h"

struct FrameReaderContext
{
    AVFormatContext *fmt;

    AVCodecContext *decoder;
    AVCodec *video_codec;

    AVPacket *pkt;
    int stream_index;

    int last_error;
    char *last_error_str;
};

LIBRARY_API(void) fr_context_free(FrameReaderContext *ctx)
{
    avcodec_free_context(&ctx->decoder);
    av_freep(&ctx->video_codec);
    avformat_free_context(ctx->fmt);
}

LIBRARY_API(int) fr_context_open(char *url, FrameReaderContext *ctx)
{
    // open input url
    ctx->last_error = avformat_open_input(&ctx->fmt, url, nullptr, nullptr);
    CHECK_ERROR(-1, ctx->last_error, ctx)

    // retrieve stream info
    ctx->last_error = avformat_find_stream_info(ctx->fmt, nullptr);
    CHECK_ERROR(-2, ctx->last_error < 0, ctx)

    // find candidate video stream
    ctx->stream_index = ctx->last_error = av_find_best_stream(ctx->fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    CHECK_ERROR(-3, ctx->last_error < 0, ctx)
    auto stream = ctx->fmt->streams[ctx->stream_index];

    // initialize decoder
    ctx->video_codec = avcodec_find_decoder(stream->codecpar->codec_id);
    ctx->decoder = avcodec_alloc_context3(ctx->video_codec);
    avcodec_parameters_to_context(ctx->decoder, stream->codecpar);
    ctx->last_error = avcodec_open2(ctx->decoder, ctx->video_codec, nullptr);
    CHECK_ERROR(-4, ctx->last_error != 0, ctx);

    // allocate receiving packet
    ctx->pkt = av_packet_alloc();

    // start playback
    av_read_play(ctx->fmt);

    return 0;
}

LIBRARY_API(int) fr_receive_frame(AVFrame *frame, FrameReaderContext *ctx)
{
    while (true)
    {
        ctx->last_error = av_read_frame(ctx->fmt, ctx->pkt);
        CHECK_ERROR(-1, ctx->last_error != 0, ctx)
        if (ctx->pkt->stream_index != ctx->stream_index)
        {
            printf("skipping frame at %lld\n", ctx->pkt->dts);
            continue; // skip non-candidate frames
        }
        printf("> av_read_frame: ok\n");

        ctx->last_error = avcodec_send_packet(ctx->decoder, ctx->pkt);
        CHECK_ERROR(-2, ctx->last_error != 0, ctx)
        printf("> avcodec_send_packet: ok\n");

        ctx->last_error = avcodec_receive_frame(ctx->decoder, frame);
        CHECK_ERROR(-3, ctx->last_error != 0, ctx)
        printf("> avcodec_receive_frame: ok\n");
        return 0;
    }
}

LIBRARY_API(void) fr_get_stream_props(int *w, int *h, FrameReaderContext *ctx)
{
    *w = ctx->decoder->width;
    *h = ctx->decoder->height;
}
