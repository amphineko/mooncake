extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

int main(int argc, char *argv[]) {
    int ret;

    if (argc <= 1) {
        fprintf(stderr, "input file expected");
        return -1;
    }

    auto file = argv[1];
    printf("trying file %s\n", file);

    // open input file
    AVFormatContext *fmt = nullptr;
    ret = avformat_open_input(&fmt, file, nullptr, nullptr);
    if (ret) {
        fprintf(stderr, "avformat_open_input: %s\n", av_err2str(ret));
        return -1;
    }
    printf("file open successfully: %s\n", argv[0]);

    // retrieve stream info
    ret = avformat_find_stream_info(fmt, nullptr);
    if (ret < 0) {
        fprintf(stderr, "avformat_find_stream_info: %s\n", av_err2str(ret));
        exit(-1);
    }
    printf("loaded stream info\n");

    // find video stream
    ret = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret < 0) {
        fprintf(stderr, "av_find_best_stream: %s\n", av_err2str(ret));
        exit(-1);
    }
    auto stream_idx = ret;
    auto stream = fmt->streams[stream_idx];
    printf("using stream #%d\n", stream_idx);

    // create decoder
    auto dec_codec = avcodec_find_decoder(stream->codecpar->codec_id);
    auto decoder = avcodec_alloc_context3(dec_codec);
    avcodec_parameters_to_context(decoder, stream->codecpar);
    ret = avcodec_open2(decoder, dec_codec, nullptr);
    if (ret != 0) {
        printf("avcodec_open2(decoder): %s\n", av_err2str(ret));
        exit(-1);
    }
    printf("opened stream #%d with codec %s\n", stream_idx, avcodec_get_name(dec_codec->id));

    // create encoder
    auto enc_codec = avcodec_find_encoder(AV_CODEC_ID_PNG);
    auto encoder = avcodec_alloc_context3(enc_codec);
    encoder->height = decoder->height;
    encoder->width = decoder->width;
    encoder->pix_fmt = AV_PIX_FMT_RGB24;
    encoder->time_base = (AVRational) {1, 1};
    ret = avcodec_open2(encoder, enc_codec, nullptr);
    if (ret != 0) {
        printf("avcodec_open2(encoder): %s\n", av_err2str(ret));
        exit(-1);
    }
    printf("create output encoder %s\n", avcodec_get_name(enc_codec->id));

    // TODO: re-mux
    int frame_count = 0;
    SwsContext *scaler = nullptr;
    auto src_frame = av_frame_alloc();
    auto pkt = av_packet_alloc();
    while (frame_count++ < 20) {
        if (av_read_frame(fmt, pkt) < 0) {
            fprintf(stderr, "av_read_frame: end of file");
            break;
        }

        if (pkt->stream_index != stream_idx) {
            printf("$%4d: skipping frame of non-candidate stream\n", frame_count);
            continue;
        }

        avcodec_send_packet(decoder, pkt);

        while (true) {
            auto recv = avcodec_receive_frame(decoder, src_frame);

            if (AVERROR(recv) == EAGAIN)
                break;

            if (recv != 0) {
                fprintf(stderr, "avcodec_receive_frame: %s (%d)\n", av_err2str(recv), recv);
                goto shutdown;
            }

            // allocate output frame
            auto frame = av_frame_alloc();
            frame->height = src_frame->height;
            frame->width = src_frame->width;
            frame->format = AV_PIX_FMT_RGB24;
            av_frame_get_buffer(frame, 32);

            // scale to output pixel format
            auto src_fmt = static_cast<AVPixelFormat >( src_frame->format);
            auto out_fmt = static_cast<AVPixelFormat >(frame->format);
            scaler = sws_getCachedContext(scaler,
                                          src_frame->width, src_frame->height, src_fmt,
                                          frame->width, frame->height, out_fmt,
                                          0, nullptr, nullptr, nullptr);
            sws_scale(scaler, src_frame->data, src_frame->linesize, 0, src_frame->height, frame->data, frame->linesize);

            // encode output frame
            auto out_pkt = av_packet_alloc();
            avcodec_send_frame(encoder, frame);
            avcodec_receive_packet(encoder, out_pkt);

            // write to file
            char out_file[64];
            sprintf(out_file, "%lld.png", pkt->pts);
            auto f = fopen(out_file, "w");
            fwrite(out_pkt->data, sizeof(char), out_pkt->size, f);
            fclose(f);
            printf("$%4d: written file %s\n", frame_count, out_file);

            // release
            av_packet_free(&out_pkt);
            av_frame_free(&frame);
        }
    }

    shutdown:
    avcodec_close(decoder);
    avcodec_free_context(&decoder);
    avformat_close_input(&fmt);
    avformat_free_context(fmt);

    return 0;
}
