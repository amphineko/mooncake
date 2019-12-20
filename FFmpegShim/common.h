#ifndef COMMON_H
#define COMMON_H

#define MAX_LAST_ERROR_STR 64

#define CHECK_ERROR(ret, fail_cond, ctx) \
    if (fail_cond) { \
        av_make_error_string(ctx->last_error_str, MAX_LAST_ERROR_STR, ctx->last_error); \
        return ret; \
    }

#define INIT_LAST_ERROR(ctx) \
    ctx->last_error = 0; \
    ctx->last_error_str = static_cast<char *>(av_mallocz(sizeof(char) * MAX_LAST_ERROR_STR));

#define FREE_LAST_ERROR(ctx) \
    av_freep(ctx->last_error_str);

#endif
