#include "library.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avio.h>
}

typedef int (*ReadCallback)(unsigned char *buf, int size);
typedef int64_t (*SeekCallback)(int64_t offset, int action);

struct StreamIOContext
{
    AVIOContext *ioCtx;

    ReadCallback read;
    SeekCallback seek;
};

int do_read(void *opaque, unsigned char *buf, int size)
{
    auto ctx = reinterpret_cast<StreamIOContext *>(opaque);
    return ctx->read(buf, size);
}

int64_t do_seek(void *opaque, int64_t offset, int whence)
{
    auto ctx = reinterpret_cast<StreamIOContext *>(opaque);
    auto action = whence | AVSEEK_SIZE ? 3 : whence & (SEEK_CUR | SEEK_END | SEEK_SET);
    return ctx->seek(offset, action);
}

#define IO_BUFFER_SIZE AV_INPUT_BUFFER_MIN_SIZE

LIBRARY_API(StreamIOContext *) sio_alloc(ReadCallback read, SeekCallback seek)
{
    auto ctx = static_cast<StreamIOContext *>(av_mallocz(sizeof(StreamIOContext)));
    ctx->read = read;
    ctx->seek = seek;

    auto buf = static_cast<unsigned char *>(av_malloc(IO_BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE));
    ctx->ioCtx = avio_alloc_context(buf, IO_BUFFER_SIZE, 0, ctx, do_read, nullptr, do_seek);

    return ctx;
}

LIBRARY_API(void) sio_close(StreamIOContext *ctx)
{
    avio_context_free(&ctx->ioCtx);
    av_free(ctx);
}

LIBRARY_API(AVIOContext *) sio_get_io_ctx(StreamIOContext *ctx)
{
    return ctx->ioCtx;
}
