# AtomicAkarin.Mooncake

libav/libswscale-based RTSP client for .NET Core

## Modules

- FFmpegShim: Handles libav/libswscale invocations and manages related structures  
- Mooncake: Initializes FFmpegShim and/or hosts frame handler plugins
- Bakery: WebM chunks in-memory *(planned)* server

## Goals and Roadmap

- [X] RTSP client via **lavf**
- [X] Receives frames via **lavc**
- [X] Convert between pixel formats via **swscale**
- [X] Encapsule **AVPacket** as **System.IO.Stream**
- [ ] ~Dump **AVFrame** into **SixLabors.ImageSharp.Image**~ *(feature not longer required)*
---
- [ ] Re-encode into **VP9** via **lavc**
- [ ] Re-mux into **WebM** via **lavf**
- [ ] Serve **WebM** chunks over HTTP (in-memory)

## Pipeline

### Input pipeline
```
lavf rtsp --> lavc frame decode --> sws convert color --> mooncake
```

### Output pipeline
```
           /--> lavc encode vp90 --> lavf mux webm --> in-memory http server
          /                                            (streaming to browser)
mooncake +
          \
           \--> imagesharp bitmap --> [bitmap handlers (plugins)]
```

## License

MIT
