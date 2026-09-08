#include <drivers/virtio/virtio.h>
#include <drivers/virtio/virtqueue.h>
#include <core/errno.h>
#include <core/kprint.h>
#include <drivers/sound/virtio_snd.h>
#include <core/mem/vmm.h>
#include <lib/string.h>
#include <core/liballoc.h>
#include <drivers/sound/audio.h>
#include <core/mem/pmm.h>

#define VSND_QUERYSZ(N, ITEMSZ) ((ITEMSZ * N) + sizeof(struct virtio_snd_hdr))

struct snd_query_buffer {
    virtio_snd_dev_t* dev;
    int desc0;
    int desc1;
    void* data;
    usize size;
    usize npgs;
};

static int vsnd_query_free(struct snd_query_buffer* buf) {
    virtqueue_free_desc(&buf->dev->queues[VIRTSND_CONTROLQ], buf->desc0);
    virtqueue_free_desc(&buf->dev->queues[VIRTSND_CONTROLQ], buf->desc1);
    vmm_unmap_pages(vmm_cpml4v(), (u64)buf->data, buf->npgs, 0);
    return 0;
}

static int vsnd_query(virtio_snd_dev_t* dev, u32 code, u32 sid, u32 n, usize itemsz, struct snd_query_buffer* buf) {
    struct virtio_snd_query_info query = {{code}, sid, n, itemsz};

    int d0 = virtqueue_alloc_desc(&dev->queues[VIRTSND_CONTROLQ]);
    int d1 = virtqueue_alloc_desc(&dev->queues[VIRTSND_CONTROLQ]);

    if (d0 < 0 || d1 < 0) return -ENOMEM;

    usize npgs = (VSND_QUERYSZ(n, itemsz) + 4095) &~ 4095;
    void* vbuf = vmm_map_pages(vmm_cpml4v(), 0, 0, npgs, MAP_CONT | MAP_ANYPHYS | MAP_ANYVIRT | MAP_CONT);
    if (!vbuf) {
        virtqueue_free_desc(&dev->queues[VIRTSND_CONTROLQ], d0);
        virtqueue_free_desc(&dev->queues[VIRTSND_CONTROLQ], d1);
        return -ENOMEM;
    }

    dev->queues[VIRTSND_CONTROLQ].desc[d0].addr = HHDM_START + vmm_get_phys(vmm_cpml4v(), (u64)&query);
    dev->queues[VIRTSND_CONTROLQ].desc[d0].len = sizeof(query);
    dev->queues[VIRTSND_CONTROLQ].desc[d0].flags = VRING_DESC_F_NEXT;
    dev->queues[VIRTSND_CONTROLQ].desc[d0].next = d1;

    dev->queues[VIRTSND_CONTROLQ].desc[d1].addr =  HHDM_START + vmm_get_phys(vmm_cpml4v(), (u64)vbuf);
    dev->queues[VIRTSND_CONTROLQ].desc[d1].len = npgs * 4096;
    dev->queues[VIRTSND_CONTROLQ].desc[d1].flags = VRING_DESC_F_WRITE;

    virtqueue_submit_chain(&dev->queues[VIRTSND_CONTROLQ], d0);
    virtqueue_kick(&dev->queues[VIRTSND_CONTROLQ]);

    u32 len;
    int used = virtqueue_poll_used(&dev->queues[VIRTSND_CONTROLQ], &len, 1000000);
    if (used < 0) {
        virtqueue_free_desc(&dev->queues[VIRTSND_CONTROLQ], d0);
        virtqueue_free_desc(&dev->queues[VIRTSND_CONTROLQ], d1);
        vmm_unmap_pages(vmm_cpml4v(), (u64)vbuf, npgs, 0);
        return used;
    }

    buf->dev = dev;
    buf->data = vbuf;
    buf->desc0 = d0;
    buf->desc1 = d1;
    buf->npgs = npgs;
    buf->size = VSND_QUERYSZ(n, itemsz);
    return 0;
}

#define CONV_FMT(FROM, TO) { if (fmt & VIRTIO_SND_PCM_FMT_##FROM) v |= PCM_FMT_##TO; }
static u64 vsnd_fmtconv(u64 fmt) {
    u64 v = 0;
    CONV_FMT(MU_LAW, MU);
    CONV_FMT(A_LAW, A);
    CONV_FMT(S8, S8);
    CONV_FMT(U8, U8);
    CONV_FMT(S16, S16);
    CONV_FMT(U16, U16);
    CONV_FMT(S18_3, S18_3);
    CONV_FMT(U18_3, U18_3);
    CONV_FMT(S20_3, S20_3);
    CONV_FMT(U20_3, U20_3);
    CONV_FMT(S24_3, S24_3);
    CONV_FMT(U24_3, U24_3);
    CONV_FMT(S20, S20);
    CONV_FMT(U20, U20);
    CONV_FMT(S24, S24);
    CONV_FMT(U24, U24);
    CONV_FMT(S32, S32);
    CONV_FMT(U32, U32);
    CONV_FMT(FLOAT, FLOAT);
    CONV_FMT(FLOAT64, FLOAT64);
    return v;
}

#define CONV_SMP(N) { if (smp & VIRTIO_SND_PCM_RATE_##N) smp |= PCM_RATE_##N; }
static u64 vsnd_smpconv(u64 smp) {
    u64 v = 0;
    CONV_SMP(5512);
    CONV_SMP(8000);
    CONV_SMP(11025);
    CONV_SMP(16000);
    CONV_SMP(22050);
    CONV_SMP(32000);
    CONV_SMP(44100);
    CONV_SMP(48000);
    CONV_SMP(64000);
    CONV_SMP(88200);
    CONV_SMP(96000);
    CONV_SMP(176400);
    CONV_SMP(192000);
    CONV_SMP(384000);
    return v;
}

int vsnd_close(audio_dev_t* self) {
    virtio_snd_dev_t* dev = self->priv;
    virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
    virtqueue_free(&dev->queues[VIRTSND_CONTROLQ]);
    virtqueue_free(&dev->queues[VIRTSND_EVENTQ]);
    virtqueue_free(&dev->queues[VIRTSND_TXQ]);
    for (usize i = 0; i < VIRTSND_N_EVTBUFS; i++) {
        pmm_ffree((void*)dev->evtbufs[i].phys, 1);
    }
    virtio_reset(&dev->dev);
    free(dev);
    return 0;
}

ssize vsnd_get_jacks(audio_dev_t* dev, audio_jack_t* jacks, usize n, usize start_id) {
    virtio_snd_dev_t* sdev = dev->priv;
    if (start_id + n > dev->njacks) n = dev->njacks - start_id;

    struct snd_query_buffer buf;
    int ret = vsnd_query(sdev, VIRTIO_SND_R_JACK_INFO, start_id, n, 
        sizeof(struct virtio_snd_jack_info), &buf);

    if (ret < 0) return ret;
    struct {
        struct virtio_snd_hdr hdr;
        struct virtio_snd_jack_info info[];
    } *data = buf.data;

    for (usize i = 0; i < n; i++) {
        jacks->conn = data->info[i].connected;
        jacks->id = i;
        jacks->dev = dev;
        jacks->priv = 0;
    }

    vsnd_query_free(&buf);
    return n;
}

ssize vsnd_get_streams(audio_dev_t* dev, audio_stream_t* streams, usize n, usize start_id) {
    virtio_snd_dev_t* sdev = dev->priv;
    if (start_id + n > dev->njacks) n = dev->njacks - start_id;

    struct snd_query_buffer buf;
    int ret = vsnd_query(sdev, VIRTIO_SND_R_PCM_INFO, start_id, n,
        sizeof(struct virtio_snd_pcm_info), &buf);

    if (ret < 0) return ret;
    struct {
        struct virtio_snd_hdr hdr;
        struct virtio_snd_pcm_info info[];
    } *data = buf.data;

    for (usize i = 0; i < n; i++) {
        streams->chmax = data->info[i].channels_max;
        streams->chmin = data->info[i].channels_min;
        streams->dir = data->info[i].direction == VIRTIO_SND_D_INPUT ? SND_STREAM_IN : SND_STREAM_OUT;
        streams->dev = dev;
        streams->id = i;
        streams->priv = 0;
        streams->fmts = vsnd_fmtconv(data->info[i].formats);
        streams->samples = vsnd_smpconv(data->info[i].rates);
    }

    vsnd_query_free(&buf);
    return n;
}

int vsnd_prepare(audio_dev_t* dev, audio_stream_t* stream, u32 bytes, u8 channels, u8 fmt, u8 rate) {
    (void)dev;(void)stream;(void)bytes;(void)channels;(void)fmt;(void)rate;
    return -1;
}

int vsnd_start(audio_dev_t* dev, audio_stream_t* stream) {
    (void)dev;(void)stream;
    return -1;
}

ssize vsnd_submit_buffer(audio_dev_t* dev, audio_stream_t* stream, void* data, usize sz) {
    (void)dev;(void)stream;(void)data;(void)sz;
    return -1;
}

int vsnd_stop(audio_dev_t* dev, audio_stream_t* stream) {
    (void)dev;(void)stream;
    return -1;
}

int virtio_snd_open(audio_dev_t* adev) {
    adev->priv = malloc(sizeof(virtio_snd_dev_t));
    if (!adev->priv) return -ENOMEM;
    virtio_snd_dev_t* dev = adev->priv;

    int ret = 0;
    if ((ret = virtio_find_pci_device(VIRTIO_DEV_SND, &dev->dev, 0)) < 0) {
        free(adev->priv);
        return ret;
    }

    virtio_reset(&dev->dev);
    virtio_set_status(&dev->dev, VIRTIO_STATUS_ACKNOWLEDGE);
    virtio_add_status(&dev->dev, VIRTIO_STATUS_DRIVER);

    u64 features = virtio_get_features64(&dev->dev);
    u64 accept = features & VIRTIO_F_VERSION_1;
    if (!(accept & VIRTIO_F_VERSION_1)) {
        kprint("virtio sound device does not support version 1 feature\n");
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        free(adev->priv);
        return -EINVAL;
    }

    virtio_set_features64(&dev->dev, accept);
    virtio_add_status(&dev->dev, VIRTIO_STATUS_FEATURES_OK);
    if (!(virtio_get_status(&dev->dev) & VIRTIO_STATUS_FEATURES_OK)) {
        kprint("feature negotiation failed for virtio sound device\n");
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        free(adev->priv);
        return -EINVAL;
    }

    if (virtqueue_init(&dev->dev, VIRTSND_CONTROLQ, &dev->queues[VIRTSND_CONTROLQ]) < 0) {
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        free(adev->priv);
        return -EINVAL;
    }

    if (virtqueue_init(&dev->dev, VIRTSND_EVENTQ, &dev->queues[VIRTSND_EVENTQ]) < 0) {
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        virtqueue_free(&dev->queues[VIRTSND_CONTROLQ]);
        free(adev->priv);
        return -EINVAL;
    }

    if (virtqueue_init(&dev->dev, VIRTSND_TXQ, &dev->queues[VIRTSND_TXQ]) < 0) {
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        virtqueue_free(&dev->queues[VIRTSND_CONTROLQ]);
        virtqueue_free(&dev->queues[VIRTSND_EVENTQ]);
        free(adev->priv);
        return -EINVAL;
    }

    if (virtqueue_init(&dev->dev, VIRTSND_RXQ, &dev->queues[VIRTSND_RXQ]) < 0) {
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        virtqueue_free(&dev->queues[VIRTSND_CONTROLQ]);
        virtqueue_free(&dev->queues[VIRTSND_EVENTQ]);
        virtqueue_free(&dev->queues[VIRTSND_TXQ]);
        free(adev->priv);
        return -EINVAL;
    }

    for (usize i = 0; i < VIRTSND_N_EVTBUFS; i++) {
        dev->evtbufs[i].phys = (u64)pmm_falloc(1);
        if (!dev->evtbufs[i].phys) {
            virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
            virtqueue_free(&dev->queues[VIRTSND_CONTROLQ]);
            virtqueue_free(&dev->queues[VIRTSND_EVENTQ]);
            virtqueue_free(&dev->queues[VIRTSND_TXQ]);
            for (usize j = 0; j < i; j++) {
                pmm_ffree((void*)dev->evtbufs[i].phys, 1);
            }
            free(adev->priv);
            return -ENOMEM;
        }

        dev->evtbufs[i].dsc = virtqueue_alloc_desc(&dev->queues[VIRTSND_EVENTQ]);
        if (dev->evtbufs[i].dsc < 0) {
            virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
            virtqueue_free(&dev->queues[VIRTSND_CONTROLQ]);
            virtqueue_free(&dev->queues[VIRTSND_EVENTQ]);
            virtqueue_free(&dev->queues[VIRTSND_TXQ]);
            for (usize j = 0; j < i; j++) {
                pmm_ffree((void*)dev->evtbufs[i].phys, 1);
            }
            free(adev->priv);
            return -ENOMEM;
        }

        dev->queues[VIRTSND_EVENTQ].desc[dev->evtbufs[i].dsc].addr = dev->evtbufs[i].phys;
        dev->queues[VIRTSND_EVENTQ].desc[dev->evtbufs[i].dsc].len = sizeof(struct virtio_snd_event);
        dev->queues[VIRTSND_EVENTQ].desc[dev->evtbufs[i].dsc].flags = VRING_DESC_F_WRITE;
        virtqueue_submit_chain(&dev->queues[VIRTSND_EVENTQ], dev->evtbufs[i].dsc);
    }

    virtqueue_kick(&dev->queues[VIRTSND_EVENTQ]);
    adev->njacks = virtio_read_config32(&dev->dev, 0);
    adev->nstreams = virtio_read_config32(&dev->dev, 4);
   
    adev->ops.get_jacks = vsnd_get_jacks;
    adev->ops.get_streams = vsnd_get_streams;
    adev->ops.close = vsnd_close;
    adev->ops.prepare = vsnd_prepare;
    adev->ops.start = vsnd_start;
    adev->ops.stop = vsnd_stop;
    adev->ops.submit_buffer = vsnd_submit_buffer;

    return 0;
}