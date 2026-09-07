#include <drivers/virtio/virtio.h>
#include <drivers/virtio/virtqueue.h>
#include <core/errno.h>
#include <core/kprint.h>
#include <drivers/sound/virtio_snd.h>
#include <core/mem/vmm.h>
#include <lib/string.h>
#include <core/liballoc.h>

#define VSND_QUERYSZ(N, ITEMSZ) (ITEMSZ * N + sizeof(struct virtio_snd_hdr))

struct snd_query_buffer {
    virtio_snd_dev_t* dev;
    int desc0;
    int desc1;
    void* data;
    usize size;
    usize npgs;
};

int vsnd_query_free(struct snd_query_buffer* buf) {
    virtqueue_free_desc(&buf->dev->queues[VIRTSND_CONTROLQ], buf->desc0);
    virtqueue_free_desc(&buf->dev->queues[VIRTSND_CONTROLQ], buf->desc1);
    vmm_unmap_pages(vmm_cpml4v(), (u64)buf->data, buf->npgs, 0);
    return 0;
}

int vsnd_query(virtio_snd_dev_t* dev, u32 code, u32 sid, u32 n, usize itemsz, struct snd_query_buffer* buf) {
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

static int vsnd_query_jacks(virtio_snd_dev_t* dev) {
    u32 jacks = virtio_read_config32(&dev->dev, 0);
    struct snd_query_buffer qbuf;

    int ret = 0;
    if ((ret = vsnd_query(dev, VIRTIO_SND_R_JACK_INFO, 0, jacks, sizeof(struct virtio_snd_jack_info), &qbuf)) < 0) {
        return ret;
    }

    struct {
        struct virtio_snd_hdr hdr;
        struct virtio_snd_jack_info info[];
    } *data = qbuf.data;

    for (usize i = 0; i < jacks; i++) {
        struct virtio_snd_jack_info* jack = &data->info[i];
        if (jack->connected) {
            if (jack->features & VIRTIO_SND_JACK_F_REMAP) {

            }
        }
    }
}

int virtio_snd_init(virtio_snd_dev_t* dev) {
    int ret = 0;
    if ((ret = virtio_find_pci_device(VIRTIO_DEV_SND, &dev->dev, 0)) < 0) {
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
        return -EINVAL;
    }
    virtio_set_features64(&dev->dev, accept);
    virtio_add_status(&dev->dev, VIRTIO_STATUS_FEATURES_OK);
    if (!(virtio_get_status(&dev->dev) & VIRTIO_STATUS_FEATURES_OK)) {
        kprint("feature negotiation failed for virtio sound device\n");
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        return -EINVAL;
    }

    if (virtqueue_init(&dev->dev, VIRTSND_CONTROLQ, &dev->queues[VIRTSND_CONTROLQ]) < 0) {
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        return -EINVAL;
    }

    if (virtqueue_init(&dev->dev, VIRTSND_EVENTQ, &dev->queues[VIRTSND_EVENTQ]) < 0) {
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        return -EINVAL;
    }

    if (virtqueue_init(&dev->dev, VIRTSND_TXQ, &dev->queues[VIRTSND_TXQ]) < 0) {
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        return -EINVAL;
    }

    if (virtqueue_init(&dev->dev, VIRTSND_RXQ, &dev->queues[VIRTSND_RXQ]) < 0) {
        virtio_set_status(&dev->dev, VIRTIO_STATUS_FAILED);
        return -EINVAL;
    }


}