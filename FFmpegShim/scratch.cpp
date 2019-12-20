extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "bmp_encoder.cpp"
#include "frame_reader.cpp"
#include "scaler.cpp"

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
    ret = bmp_context_open(encoder, AV_CODEC_ID_PNG, src_width, src_height, AV_PIX_FMT_RGB24);
    if (ret)
    {
        fprintf(stderr, "bmp_context_open (step %d): %s (%d)\n", ret, encoder->last_error_str, encoder->last_error);
        return -1;
    }
    printf("bmp_context_open: ok\n");

    // TODO: re-mux
    auto scaler = static_cast<ScalerContext *>(av_mallocz(sizeof(ScalerContext)));
    auto src_frame = av_frame_alloc();

    for (auto i = 0; i < 20; i++)
    {
        ret = fr_receive_frame(src_frame, reader);
        if (ret != 0)
        {
            fprintf(stderr, "fr_receive_frame (step %d): %s (%d)", ret, reader->last_error_str, reader->last_error);
            goto shutdown;
        }

        auto frame = av_frame_alloc();
        scaler->out_h = frame->height = src_frame->height;
        scaler->out_w = frame->width = src_frame->width;
        scaler->out_fmt = frame->format = AV_PIX_FMT_RGB24;
        av_frame_get_buffer(frame, 32);
        scaler_scale(src_frame, frame, scaler);

        auto out_pkt = av_packet_alloc();
        scaler_scale(src_frame, frame, scaler);

        char out_file[64];
        sprintf(out_file, "%d.png", i);
        auto f = fopen(out_file, "w");
        fwrite(out_pkt->data, sizeof(char), out_pkt->size, f);
        fclose(f);
        printf("$%4d: written file %s\n", i, out_file);

        av_packet_free(&out_pkt);
        av_frame_free(&frame);
    }

shutdown:
    bmp_context_free(encoder);
    scaler_free(scaler);
    fr_context_free(reader);

    return 0;
}
