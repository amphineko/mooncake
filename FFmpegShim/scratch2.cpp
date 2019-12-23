extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

#include "bmp_encoder.cpp"
#include "frame_reader.cpp"
#include "scaler.cpp"

const AVCodecID OUT_ENCODER_ID = AV_CODEC_ID_PNG;
const AVPixelFormat OUT_PIXEL_FORMAT = AV_PIX_FMT_RGB24;

char *make_error(int err_num)
{
    size_t BUF_SIZE = 64;
    auto buf = static_cast<char *>(av_mallocz(BUF_SIZE));
    av_make_error_string(buf, BUF_SIZE, err_num);
}

#define CHECK_FUNCTION(func_name, fail_cond)                                                                           \
    if (fail_cond)                                                                                                     \
    {                                                                                                                  \
        auto err_buf = make_error(ret);                                                                                \
        printf("%s: %s (%d)\n", func_name, err_buf, ret);                                                              \
        av_freep(&err_buf);                                                                                            \
        return -1;                                                                                                     \
    }

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

    // open input file/url
    auto fmt = static_cast<AVFormatContext *>(nullptr);
    ret = avformat_open_input(&fmt, file, nullptr, nullptr);
    CHECK_FUNCTION("avformat_open_input", ret != 0)

    // retrieve stream info
    ret = avformat_find_stream_info(fmt, nullptr);
    CHECK_FUNCTION("avformat_find_stream_info", ret < 0)

    // find candidate stream
    ret = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    CHECK_FUNCTION("av_find_best_stream", ret < 0)
    auto stream_idx = ret;
    auto stream = fmt->streams[stream_idx];

    // create decoder
    auto dec_par = stream->codecpar;
    auto dec_id = dec_par->codec_id;
    printf("using codec %s (%d)\n", avcodec_get_name(dec_id), dec_id);
    auto dec_codec = avcodec_find_decoder(dec_id);
    auto dec = avcodec_alloc_context3(dec_codec);
    avcodec_parameters_to_context(dec, dec_par);

    // open decoder
    ret = avcodec_open2(dec, dec_codec, nullptr);
    CHECK_FUNCTION("avcodec_open2(decoder)", ret != 0)
    char dec_pix_fmt_str[32];
    printf("avcodec_open2(decoder): ok w=%d h=%d pix_fmt=%s\n", dec->width, dec->height,
           av_get_pix_fmt_string(dec_pix_fmt_str, 32, dec->pix_fmt));

    // create encoder
    auto enc_codec = avcodec_find_encoder(OUT_ENCODER_ID);
    auto enc = avcodec_alloc_context3(enc_codec);
    enc->height = dec->height;
    enc->width = dec->width;
    enc->pix_fmt = OUT_PIXEL_FORMAT;
    enc->time_base = {1, 1};

    // open encoder
    ret = avcodec_open2(enc, enc_codec, nullptr);
    CHECK_FUNCTION("avcodec_open2(encoder)", ret != 0)
    char enc_pix_fmt_str[32];
    printf("avcodec_open2(encoder): ok w=%d h=%d pix_fmt=%s\n", enc->width, enc->height,
           av_get_pix_fmt_string(enc_pix_fmt_str, 32, enc->pix_fmt));

    // start playback
    av_read_play(fmt);

    auto frame_count = 0;
    auto scaler = static_cast<SwsContext *>(nullptr);
    while (frame_count < 20)
    {
        // read raw packet
        auto pkt = av_packet_alloc();
        ret = av_read_frame(fmt, pkt);
        CHECK_FUNCTION("av_read_frame", ret != 0)
        if (pkt->stream_index != stream_idx)
        {
            av_packet_free(&pkt);
            continue;
        }

        // feed decoder with raw packet
        ret = avcodec_send_packet(dec, pkt);
        CHECK_FUNCTION("avcodec_send_packet", ret != 0)
        printf("avcodec_send_packet: ok pos=%lld\n", pkt->pos);
        av_packet_free(&pkt);

        // transcode frames
        while (frame_count < 20)
        {
            // receive decoded frame
            auto frame = av_frame_alloc();
            ret = avcodec_receive_frame(dec, frame);
            if (ret == AVERROR(EAGAIN))
            {
                av_frame_free(&frame);
                break;
            }
            CHECK_FUNCTION("avcodec_receive_frame", ret != 0)
            char pix_fmt_str[32] = {0};
            printf("avcodec_receive_frame: ok %lld w=%d h=%d pix_fmt=%s\n", frame->pkt_dts, frame->width, frame->height,
                   av_get_pix_fmt_string(pix_fmt_str, 32, static_cast<AVPixelFormat>(frame->format)));

            // create output frame
            auto out = av_frame_alloc();
            out->height = frame->height;
            out->width = frame->width;
            out->format = OUT_PIXEL_FORMAT;
            av_frame_get_buffer(out, 32);

            // transform pixel format
            scaler = sws_getCachedContext(scaler, frame->width, frame->height,
                                          static_cast<AVPixelFormat>(frame->format), out->width, out->height,
                                          static_cast<AVPixelFormat>(out->format), 0, nullptr, nullptr, nullptr);
            sws_scale(scaler, frame->data, frame->linesize, 0, frame->height, out->data, out->linesize);
            av_frame_free(&frame);

            // re-encode
            avcodec_send_frame(enc, out);
            av_frame_free(&out);

            // write out
            auto out_pkt = av_packet_alloc();
            avcodec_receive_packet(enc, out_pkt);

            char filename[32];
            sprintf(filename, "frame-%d.png", frame_count);
            auto f = fopen(filename, "wb");
            fwrite(out_pkt->data, sizeof(out_pkt->data[0]), out_pkt->size, f);
            av_packet_free(&out_pkt);
            fclose(f);
            printf("fwrite(%s): ok\n", filename);

            frame_count++;
        }
    }

    // shutdown
    sws_freeContext(scaler);
    avcodec_free_context(&dec);
    avformat_close_input(&fmt);

    return 0;
}
