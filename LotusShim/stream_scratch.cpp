#include <cstdio>
#include <fstream>

extern "C"
{
#include <libavutil/common.h>
#include <libavutil/error.h>
}

#include "frame_encoder.cpp"
#include "frame_reader.cpp"
#include "frame_scaler.cpp"
#include "stream.cpp"

int read_stream(void *opaque, unsigned char *buf, int size)
{
    auto stream = reinterpret_cast<std::ifstream *>(opaque);

    printf("read pre:\tpos=%d\tsize=%d\n", (int)stream->tellg(), size);
    auto read_count = stream->read(reinterpret_cast<char *>(buf), size).gcount();
    printf("read post:\tpos=%d\tgcount=%lld\n", (int)stream->tellg(), read_count);

    return read_count;
}

long long seek_stream(void *opaque, long long offset, int whence)
{
    auto stream = reinterpret_cast<std::ifstream *>(opaque);

    if (whence | AVSEEK_SIZE)
    {
        auto cur = (long long)stream->tellg(); // preserve current position

        stream->seekg(0, std::ios::end);        // seek to the end
        auto size = (long long)stream->tellg(); // retrieve position (which is length of stream)
        stream->seekg(cur);                     // seek back to original position

        printf("seek size: size=%lld\tpos=%lld\n", size, cur);
        return size;
    }

    auto action = (whence & (SEEK_CUR | SEEK_END | SEEK_SET));
    printf("seek pre:\tpos=%d\toffset=%lld\taction=%d\n", (int)stream->tellg(), offset, action);

    switch (action)
    {
    case SEEK_CUR:
        stream->seekg(offset, std::ios::cur);
        break;
    case SEEK_END:
        stream->seekg(offset, std::ios::end);
        break;
    case SEEK_SET:
        stream->seekg(offset, std::ios::beg);
        break;
    default:
        return -1;
    }

    printf("seek post:\tpos=%d\n", (int)stream->tellg());
    return (long)stream->tellg();
}

#define EXIT_ERROR(ret, fail_cond, step)                                                                               \
    if (fail_cond)                                                                                                     \
    {                                                                                                                  \
        char err_buf[64];                                                                                              \
        av_make_error_string(err_buf, 64, ret);                                                                        \
        printf("%s: %s %d\n", step, err_buf, ret);                                                                     \
    }                                                                                                                  \
    else                                                                                                               \
        printf("%s: ok (%d)\n", step, ret);

#define OUTPUT_CODEC_ID AV_CODEC_ID_MJPEG
#define OUTPUT_PIXEL_FORMAT AV_PIX_FMT_YUVJ420P

int main(int arg_count, char *args[])
{
    int ret;

    if (arg_count < 2)
    {
        fprintf(stderr, "Missing filename (%d)", arg_count);
        return -1;
    }
    auto filename = args[1];

    std::ifstream stream;
    stream.open(filename, std::ios::binary);
    stream.seekg(0);
    printf("stream.open: ok\n");

    auto buf = static_cast<unsigned char *>(av_mallocz(AV_INPUT_BUFFER_MIN_SIZE + AV_INPUT_BUFFER_PADDING_SIZE));
    auto io = avio_alloc_context(buf, AV_INPUT_BUFFER_MIN_SIZE, 0, &stream, read_stream, nullptr, seek_stream);

    auto reader = fr_context_alloc();
    ret = fr_context_open2(io, filename, reader);
    EXIT_ERROR(ret, ret != 0, "fr_context_open2");

    int width, height, pix_fmt;
    const char *codec_name;
    fr_get_video_props(&width, &height, &pix_fmt, &codec_name, reader);
    printf("fr_get_video_props: w%d h%d pix_fmt%d codec_name=%s\n", width, height, pix_fmt, codec_name);

    auto encoder = encoder_context_create();
    encoder_context_open(OUTPUT_CODEC_ID, width, height, OUTPUT_PIXEL_FORMAT, encoder);

    auto scaler = scaler_context_alloc();

    for (auto i = 0; i <= 1000; i += 1)
    {
        auto frame = av_frame_alloc();
        ret = fr_read_frame(frame, reader);
        if (ret != 0)
        {
            // prevent unnecessary print
            EXIT_ERROR(ret, ret != 0, "fr_read_frame")
        }

        if (i % 100 != 0)
        {
            av_frame_free(&frame);
            continue;
        }

        auto out = av_frame_alloc();
        out->width = frame->width;
        out->height = frame->height;
        out->format = OUTPUT_PIXEL_FORMAT;
        av_frame_get_buffer(out, 32);
        scaler_scale(out, frame, scaler);
        av_frame_free(&frame);

        auto pkt = av_packet_alloc();
        ret = encoder_encode(pkt, out, encoder);
        EXIT_ERROR(ret, ret != 0, "encoder_encode");
        av_frame_free(&out);

        char out_filename[64];
        sprintf(out_filename, "frame-%d.jpg", i);
        auto f = fopen(out_filename, "wb");
        fwrite(pkt->data, sizeof(char), pkt->size, f);
        fclose(f);
    }

    fr_context_close(reader);

    return 0;
}