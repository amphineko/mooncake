extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "export.h"

using frame_callback_t = void (*)();

struct DecoderContext
{
    AVCodec *codec;
    AVCodecContext *codec_ctx;
    AVCodecParserContext *parser;

    AVFrame *frame;
    AVPacket *packet;
};

LIBRARY_API(int) decoder_version()
{
    return 0;
}

LIBRARY_API(void) decoder_context_dispose(DecoderContext *ctx)
{
    av_packet_free(&ctx->packet);
    av_frame_free(&ctx->frame);

    av_parser_close(ctx->parser);
    av_freep(&ctx->parser);

    avcodec_free_context(&ctx->codec_ctx);
    av_freep(&ctx->codec);
}

LIBRARY_API(int) decoder_context_init(int codec_id, DecoderContext *ctx)
{
    ctx->codec = avcodec_find_decoder(static_cast<AVCodecID>(codec_id));
    if (!ctx->codec)
        return -1;

    ctx->codec_ctx = avcodec_alloc_context3(ctx->codec);
    if (!ctx->codec_ctx)
        return -2;

    if (avcodec_open2(ctx->codec_ctx, ctx->codec, nullptr) < 0)
        return -3;

    ctx->parser = av_parser_init(ctx->codec->id);

    ctx->frame = av_frame_alloc();
    ctx->packet = av_packet_alloc();
    return 0;
}

LIBRARY_API(int) decoder_receive_frame(DecoderContext *ctx)
{
    return avcodec_receive_frame(ctx->codec_ctx, ctx->frame);
}

LIBRARY_API(int) decoder_send_packet(uint8_t *buf, size_t size, DecoderContext *ctx)
{
    auto pkt = av_packet_alloc();
    pkt->data = buf;
    pkt->size = size;
    auto ret = avcodec_send_packet(ctx->codec_ctx, pkt);
    av_packet_free(&pkt);
    return ret;
}

LIBRARY_API(int) decoder_set_extra_data(char *data, size_t size, DecoderContext *ctx)
{
    if (ctx->codec_ctx->extradata && ctx->codec_ctx->extradata_size < size)
        av_freep(ctx->codec_ctx->extradata);

    if (!ctx->codec_ctx->extradata)
        ctx->codec_ctx->extradata = static_cast<uint8_t *>(av_malloc(size + AV_INPUT_BUFFER_PADDING_SIZE));

    ctx->codec_ctx->extradata_size = size;
    memcpy(ctx->codec_ctx->extradata, data, size);
    memset(ctx->codec_ctx->extradata + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    avcodec_close(ctx->codec_ctx);
    if (avcodec_open2(ctx->codec_ctx, ctx->codec, nullptr) < 0)
        return -1;

    return 0;
}
