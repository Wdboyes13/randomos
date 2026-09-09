#include <core/errno.h>
#include <drivers/sound/audio.h>
#include <drivers/sound/virtio_snd.h>
#include <core/liballoc.h>
#include <core/kprint.h>

audio_dev_t* open_sound(int type) {
    audio_dev_t* dev = malloc(sizeof(*dev));
    if (!dev) return NULL;

    if (type == SNDDEV_VIRTIO) {
        if (virtio_snd_open(dev) < 0) {
            kprint("Failed to initialize virtio sound device\n");
            return NULL;
        }

        audio_jack_t* jacks = malloc(sizeof(*jacks) * dev->njacks);
        if (!jacks) {
            kprint("Failed to allocate jacks\n");
            return NULL;
        }

        ssize gotten = dev->ops.get_jacks(dev, jacks, dev->njacks, 0);
        if (gotten < 0) {
            kprint("Failed to get jacks\n");
            free(jacks);
            return NULL;
        }

        for (ssize i = 0; i < gotten; i++ ) {
            if (jacks[i].conn) {
                kprint("Jack %d connected\n", jacks[i].id);
            }
        }
        free(jacks);

        audio_stream_t* streams = malloc(sizeof(*streams) * dev->nstreams);
        if (!streams) {
            kprint("Failed to allocate streams\n");
            return NULL;
        }

        gotten = dev->ops.get_streams(dev, streams, dev->nstreams, 0);
        if (gotten < 0) {
            kprint("Failed to get streams\n");
            free(streams);
            return NULL;
        }

        for (ssize i = 0; i < gotten; i++) {
            kprint("Stream %ld\n", i);
            kprint("    Sample support: %016lx\n", streams[i].samples);
            kprint("    Direction: %s\n", streams[i].dir == SND_STREAM_IN ? "IN" : "OUT");
            kprint("    Min streams: %08x\n", streams[i].chmin);
            kprint("    Max streams: %08x\n", streams[i].chmax);
            kprint("    Formats supported: %016lx\n", streams[i].fmts);
        }

        free(streams);
        kprint("VirtIO Sound Succeeded\n");
        return dev;
    } else {
        return NULL;
    }
}