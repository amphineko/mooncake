#ifndef EXPORT_HELPER_H
#define EXPORT_HELPER_H

#ifdef _MSC_VER
#define LIBRARY_API(ret) extern "C" __declspec(dllexport) ret __stdcall
#endif

#ifdef __GNUC__
#define LIBRARY_API(ret) extern "C" __attribute__((visibility("default"))) ret
#endif

#define MAX_LAST_ERROR_STR 128

#define CHECK_LAV_ERROR(fail_ret, fail_cond, step, ctx)                                                                \
    if (fail_cond)                                                                                                     \
    {                                                                                                                  \
        av_make_error_string(ctx->last_error_str, MAX_LAST_ERROR_STR, ctx->last_error);                                \
        strcpy(ctx->last_error_step, step);                                                                            \
        return fail_ret;                                                                                               \
    }

#define CHECK_LAV_ERROR_LE_ZERO(fail_ret, step, ctx) CHECK_LAV_ERROR(fail_ret, ctx->last_error < 0, step, ctx)

#define CHECK_LAV_ERROR_NON_ZERO(fail_ret, step, ctx) CHECK_LAV_ERROR(fail_ret, ctx->last_error != 0, step, ctx)

#define DEFINE_LAST_ERROR                                                                                              \
    int last_error;                                                                                                    \
    char last_error_str[MAX_LAST_ERROR_STR];                                                                           \
    char last_error_step[MAX_LAST_ERROR_STR];

#define INIT_LAST_ERROR(ctx)                                                                                           \
    ctx->last_error = 0;                                                                                               \
    memset(ctx->last_error_str, 0, sizeof(char) * MAX_LAST_ERROR_STR);

#endif // EXPORT_HELPER_H
