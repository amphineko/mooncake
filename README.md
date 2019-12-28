# AtomicAkarin.Mooncake

libav/libswscale-based RTSP client for .NET Core

This project is still under development, you may check out other branches for latest progress.

## Modules

- Lotus(Core/Shim): Handles libav/libswscale invocations and manages related structures  
- Mooncake: Initializes FFmpegShim and/or hosts frame handler plugins
- Bakery: WebM chunks in-memory _(planned)_ server

## Goals and Roadmap

- [X] **FrameReader**: Open supported media files or urls _(e.g rtsp://)_ via **avformat**

- [X] **FrameReader**: Receive and decode frames via **avcodec**

---

- [X] **FrameScaler**: Convert frames into different pixel formats _(e.g YUV420P to RGB24)_ via **swscale**

---

- [ ] ~~Dump **AVFrame** into **SixLabors.ImageSharp.Image**~~ _(feature not longer required)_

- [X] **AVPacketStream**: Encapulse **AVPacket** as **System.IO.Stream** _(read-only)_
> Individual packets can be dumped out (e.g re-encode frames as JPEG and dump into .NET) as simple as `Stream.CopyTo()`.

- [ ] **StreamIOContext**: Encapsule **System.IO.Stream** _(read and write)_ as **AVIOContext**
> **FileStream** and **MemoryStream** can be provided to **AVFormat** as input source or output destination, which enables in-memory re-muxing.

---

- [ ] Re-encode and mux into **VP9 WebM**

- [ ] Serve output **WebM** _(cached in in-memory)_ chunks over HTTP

## License

MIT
