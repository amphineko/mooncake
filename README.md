# AtomicAkarin.Mooncake

libav/libswscale-based RTSP client for .NET Core

- FFmpegShim: Handles libav/libswscale invocations and manages related structures  
- Mooncake: Initializes FFmpegShim and/or hosts frame handler plugins
- Bakery: WebM chunks in-memory *(planned)* server

## Goals and Roadmap

- [X] RTSP client via **lavf**
- [ ] Receives frames via **lavc**
- [ ] Convert colorspace into RGB via **swscale**
- [ ] Dump **AVFrame** into **SixLabors.ImageSharp.Image**
---
- [ ] Re-encode into **VP9** via **lavc**
- [ ] Re-mux into **WebM** via **lavf**
- [ ] Serve **WebM** chunks over HTTP (in-memory)

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
