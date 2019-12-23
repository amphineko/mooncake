extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "bmp_encoder.cpp"
#include "frame_reader.cpp"
#include "scaler.cpp"

const AVCodecID OUT_CODEC_ID = AV_CODEC_ID_PNG;
const AVPixelFormat OUT_PIXEL_FORMAT = AV_PIX_FMT_RGB24;

int main(int argc, char *argv[])
{
    int ret;

    if (argc <= 1)
    {
        fprintf(stderr, "input file expected");
        return -1;
    }

    auto file = argv[1];
    printf("trying file %s\n", file);

    auto reader = static_cast<FrameReaderContext *>(av_mallocz(sizeof(FrameReaderContext)));
    ret = fr_context_open(file, reader);
    if (ret)
    {
        fprintf(stderr, "fr_context_open (step %d): %s (%d)\n", ret, reader->last_error_str, reader->last_error);
        return -1;
    }
    printf("fr_context_open: ok\n");

    auto encoder = static_cast<BmpEncoderContext *>(av_mallocz(sizeof(BmpEncoderContext)));
    int src_width, src_height;
    fr_get_stream_props(&src_width, &src_height, reader);
    ret = bmp_context_open(encoder, OUT_CODEC_ID, src_width, src_height, OUT_PIXEL_FORMAT);
    if (ret)
    {
        fprintf(stderr, "bmp_context_open (step %d): %s (%d)\n", ret, encoder->last_error_str, encoder->last_error);
        return -1;
    }
    printf("bmp_context_open: ok\n");

    auto scaler = static_cast<ScalerContext *>(av_mallocz(sizeof(ScalerContext)));
    for (auto i = 0; i < 20; i++)
    {
        auto src = av_frame_alloc();
        ret = fr_receive_frame(src, reader);
        if (ret != 0)
        {
            fprintf(stderr, "fr_receive_frame (step %d): %s (%d)", ret, reader->last_error_str, reader->last_error);
            goto shutdown;
        }

        auto out = av_frame_alloc();
        out->height = src->height;
        out->width = src->width;
        out->format = OUT_PIXEL_FORMAT;
        av_frame_get_buffer(out, 32);

        scaler_scale(src, out, scaler);
        av_frame_free(&src);

        auto pkt = av_packet_alloc();
        bmp_encode(out, pkt, encoder);
        av_frame_free(&out);

        char out_file[64];
        sprintf(out_file, "frame-%d.png", i);
        auto f = fopen(out_file, "w");
        fwrite(pkt->data, sizeof(char), pkt->size, f);
        fclose(f);
        av_packet_free(&pkt);

        printf("$%4d: written file %s\n", i, out_file);
    }

shutdown:
    bmp_context_free(encoder);
    scaler_free(scaler);
    fr_context_free(reader);

    return 0;
}
