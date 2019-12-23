#include "common.h"
#include "export.h"

struct FrameReaderContext
{
    AVFormatContext *fmt;

    AVCodecContext *dec;
    AVCodec *dec_codec;

    int stream_index;

    int last_error;
    char *last_error_str;
};

void fr_dump_format(FrameReaderContext *ctx)
{
    av_dump_format(ctx->fmt, ctx->stream_index, nullptr, 0);
}

LIBRARY_API(void) fr_context_free(FrameReaderContext *ctx)
{
    avcodec_free_context(&ctx->dec);
    avformat_free_context(ctx->fmt);
}

LIBRARY_API(int) fr_context_open(char *url, FrameReaderContext *ctx)
{
    // open input url
    ctx->fmt = nullptr;
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
    auto dec_par = stream->codecpar;
    auto dec_id = dec_par->codec_id;
    ctx->dec_codec = avcodec_find_decoder(dec_id);
    ctx->dec = avcodec_alloc_context3(ctx->dec_codec);
    avcodec_parameters_to_context(ctx->dec, dec_par);

    ctx->last_error = avcodec_open2(ctx->dec, ctx->dec_codec, nullptr);
    CHECK_ERROR(-4, ctx->last_error != 0, ctx);

    // start playback
    av_read_play(ctx->fmt);

    return 0;
}

LIBRARY_API(int) fr_receive_frame(AVFrame *frame, FrameReaderContext *ctx)
{
    while (true)
    {
        // try to receive buffered decoded frame
        ctx->last_error = avcodec_receive_frame(ctx->dec, frame);
        if (ctx->last_error == 0)
            return 0; // got new frame
        CHECK_ERROR(-3, ctx->last_error != AVERROR(EAGAIN), ctx)

        // read/receive raw packet
        auto pkt = av_packet_alloc();
        ctx->last_error = av_read_frame(ctx->fmt, pkt);
        CHECK_ERROR(-1, ctx->last_error != 0, ctx)
        if (pkt->stream_index != ctx->stream_index)
            goto skip_frame; // skip non-candidate frames

        // feed raw packet to decoder
        ctx->last_error = avcodec_send_packet(ctx->dec, pkt);
        CHECK_ERROR(-2, ctx->last_error != 0, ctx)

    skip_frame:
        av_packet_free(&pkt);
    }
}

LIBRARY_API(void) fr_get_stream_props(int *w, int *h, FrameReaderContext *ctx)
{
    *w = ctx->dec->width;
    *h = ctx->dec->height;
}
