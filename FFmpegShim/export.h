#ifndef FFMPEGSHIM_EXPORT_H
#define FFMPEGSHIM_EXPORT_H

#define LIBRARY_API(ret_type) extern "C" __declspec(dllexport) ret_type __stdcall

#endif
