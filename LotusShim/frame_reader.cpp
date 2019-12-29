#include "library.h"

extern "C"
{
#include <libavformat/avformat.h>
}

struct frame_reader_ctx_t
{
    DEFINE_LAST_ERROR

    AVFormatContext *fmt;
    AVCodecContext *dec;

    int stream_index;
};

LIBRARY_API(frame_reader_ctx_t *) fr_context_alloc()
{
    return static_cast<frame_reader_ctx_t *>(av_mallocz(sizeof(frame_reader_ctx_t)));
}

LIBRARY_API(void) fr_context_close(frame_reader_ctx_t *ctx)
{
    avcodec_free_context(&ctx->dec);
    avformat_free_context(ctx->fmt);
    av_free(ctx);
}

LIBRARY_API(int) fr_context_open2(AVIOContext *ioCtx, char *url, frame_reader_ctx_t *ctx)
{
    INIT_LAST_ERROR(ctx)

    // open input url
    auto fmt = avformat_alloc_context();
    fmt->pb = ioCtx;
    ctx->last_error = avformat_open_input(&fmt, url, nullptr, nullptr);
    CHECK_LAV_ERROR_NON_ZERO(-1, "avformat_open_input", ctx)
    ctx->fmt = fmt;

    // retrieve stream info
    ctx->last_error = avformat_find_stream_info(fmt, nullptr);
    CHECK_LAV_ERROR_LE_ZERO(-2, "avformat_find_stream_info", ctx)

    // find candidate video stream
    auto stream_index = ctx->last_error = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    CHECK_LAV_ERROR_LE_ZERO(-3, "av_find_best_stream", ctx)
    ctx->stream_index = stream_index;
    auto stream = fmt->streams[stream_index];

    // initialize decoder
    auto dec_par = stream->codecpar;
    auto dec_codec = avcodec_find_decoder(dec_par->codec_id);
    auto dec = avcodec_alloc_context3(dec_codec);
    CHECK_LAV_ERROR(-4, dec == nullptr, "avcodec_alloc_context3", ctx)
    ctx->dec = dec;
    avcodec_parameters_to_context(dec, dec_par);

    // open decoder
    ctx->last_error = avcodec_open2(dec, dec_codec, nullptr);
    CHECK_LAV_ERROR_NON_ZERO(-5, "avcodec_open2", ctx)

    return 0;
}

LIBRARY_API(int) fr_context_open(char *url, frame_reader_ctx_t *ctx)
{
    return fr_context_open2(nullptr, url, ctx);
}

LIBRARY_API(void) fr_get_video_props(int *w, int *h, int *pix_fmt, const char **codec_name, frame_reader_ctx_t *ctx)
{
    auto dec = ctx->dec;
    *w = dec->width;
    *h = dec->height;
    *pix_fmt = dec->pix_fmt;
    *codec_name = avcodec_get_name(dec->codec_id);
}

LIBRARY_API(int) fr_read_frame(AVFrame *out, frame_reader_ctx_t *ctx)
{
    auto dec = ctx->dec;
    auto fmt = ctx->fmt;
    auto stream_index = ctx->stream_index;

    while (true)
    {
        // try receiving one decoded frame
        ctx->last_error = avcodec_receive_frame(dec, out);
        if (ctx->last_error == 0) // got one frame from decoder
            return 0;
        if (ctx->last_error != AVERROR(EAGAIN))
            CHECK_LAV_ERROR_NON_ZERO(-1, "avcodec_receive_frame", ctx)

        // read raw packet
        auto pkt = av_packet_alloc();
        ctx->last_error = av_read_frame(fmt, pkt);
        CHECK_LAV_ERROR_NON_ZERO(-2, "av_read_frame", ctx)
        if (pkt->stream_index != stream_index) // skip non-candidate stream (e.g audio stream)
            continue;

        // feed decoder with raw packet
        ctx->last_error = avcodec_send_packet(dec, pkt);
        CHECK_LAV_ERROR_NON_ZERO(-3, "avcodec_send_packet", ctx)
        av_packet_free(&pkt);
    }
}
