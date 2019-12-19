#ifndef FFMPEGSHIM_EXPORT_H
#define FFMPEGSHIM_EXPORT_H

#if defined(_MSC_VER)
#define LIBRARY_API(ret) __declspec(dllexport) ret __stdcall
#elif defined(__GNUC__)
#define LIBRARY_API(ret) __attribute__((visibility("default"))) ret
#endif

#endif
