#include "export.h"

extern "C"
{
#include <libavutil/frame.h>
#include <libavutil/mem.h>
}

LIBRARY_API(AVFrame *) shim_alloc_frame()
{
    return av_frame_alloc();
}

LIBRARY_API(void) shim_free(void *ptr)
{
    av_free(ptr);
}
