#include <cstdio>
#include <fstream>

extern "C"
{
#include <libavutil/common.h>
#include <libavutil/error.h>
}

#include "frame_reader.cpp"
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

    fr_context_close(reader);

    return 0;
}