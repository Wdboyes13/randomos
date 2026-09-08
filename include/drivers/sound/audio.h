#pragma once
#include <core/std.h>

// if user provides data in a format unsupported by
// the device, kernel should try to convert it to a supported
// format if possible

typedef enum {
    PCM_FMT_MU,
    PCM_FMT_A,
    PCM_FMT_S8,
    PCM_FMT_U8,
    PCM_FMT_S16,
    PCM_FMT_U16,
    PCM_FMT_S18_3,
    PCM_FMT_U18_3,
    PCM_FMT_S20_3,
    PCM_FMT_U20_3,
    PCM_FMT_S24_3,
    PCM_FMT_U24_3,
    PCM_FMT_S20,
    PCM_FMT_U20,
    PCM_FMT_S24,
    PCM_FMT_U24,
    PCM_FMT_S32,
    PCM_FMT_U32,
    PCM_FMT_FLOAT,
    PCM_FMT_FLOAT64
} audio_fmt_t;

typedef enum {
    PCM_RATE_5512,
    PCM_RATE_8000,
    PCM_RATE_11025,
    PCM_RATE_16000,
    PCM_RATE_22050,
    PCM_RATE_32000,
    PCM_RATE_44100,
    PCM_RATE_48000,
    PCM_RATE_64000,
    PCM_RATE_88200,
    PCM_RATE_96000,
    PCM_RATE_176400,
    PCM_RATE_192000,
    PCM_RATE_384000
} audio_rate_t;

typedef struct AudioDeviceType audio_dev_t;

typedef struct {
    u32 id;
    bool conn;
    audio_dev_t* dev;
    void* priv;
} audio_jack_t;

typedef struct {
    u32 id; // id of stream
    u64 samples; // bitmask of supported sample rates
    enum {
        SND_STREAM_IN,
        SND_STREAM_OUT
    } dir;
    u32 chmin; // minimum channels
    u32 chmax; // max channels
    u64 fmts; // bitmask of supported formats
    audio_dev_t* dev;
    void* priv;
} audio_stream_t;

// these should be provided by the backend driver
// soon we should have some sort of callback system for when the
// device needs more data
typedef struct {
    int (*close)(audio_dev_t* self);
    ssize (*get_jacks)(audio_dev_t* dev, audio_jack_t* jacks, usize n, usize start_id); // gets n audio jacks starting at id start_id
    ssize (*get_streams)(audio_dev_t* dev, audio_stream_t* streams, usize n, usize start_id); // gets n streams starting at id start_id
    int (*prepare)(audio_dev_t* dev, audio_stream_t* stream, u32 bytes, u8 channels, u8 fmt, u8 rate); // configures and prepares a device for playback
    int (*start)(audio_dev_t* dev, audio_stream_t* stream); // starts playback on the stream
    ssize (*submit_buffer)(audio_dev_t* dev, audio_stream_t* stream, void* data, usize sz); // submits a buffer to the device
    int (*stop)(audio_dev_t* dev, audio_stream_t* stream); // stops playback on the stream
} audio_ops_t;

int open_audio(audio_dev_t* dev);

struct AudioDeviceType {
    usize njacks;
    usize nstreams;
    audio_ops_t ops;
    void* priv;
};