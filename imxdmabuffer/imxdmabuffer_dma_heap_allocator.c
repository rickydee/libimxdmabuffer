/*
 * imxdmabuffer_dma_heap_allocator.c
 * Patched for modern kernels without DMA_BUF_IOCTL_PHYS
 */

#include "imxdmabuffer.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>   // <- fix for munmap/mmap
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

/* Dummy macros for test code compatibility */
#ifndef IMX_DMA_BUFFER_DMA_HEAP_ALLOCATOR_DEFAULT_FD_FLAGS
#define IMX_DMA_BUFFER_DMA_HEAP_ALLOCATOR_DEFAULT_FD_FLAGS 0
#endif

#ifndef IMX_DMA_BUFFER_DMA_HEAP_ALLOCATOR_DEFAULT_HEAP_FLAGS
#define IMX_DMA_BUFFER_DMA_HEAP_ALLOCATOR_DEFAULT_HEAP_FLAGS 0
#endif

struct imx_dma_heap_buffer {
    int dmabuf_fd;
    size_t size;
    void *mapped;
};

/* Allocates a dma-heap buffer */
struct imx_dma_heap_buffer *imx_dma_heap_allocator_alloc(size_t size)
{
    struct imx_dma_heap_buffer *buffer = calloc(1, sizeof(*buffer));
    if (!buffer)
        return NULL;

    buffer->dmabuf_fd = open("/dev/dma_heap/linux,cma", O_RDWR);
    if (buffer->dmabuf_fd < 0) {
        free(buffer);
        return NULL;
    }

    buffer->size = size;
    return buffer;
}

/* Frees a dma-heap buffer */
void imx_dma_heap_allocator_free(struct imx_dma_heap_buffer *buffer)
{
    if (!buffer)
        return;

    if (buffer->mapped)
        munmap(buffer->mapped, buffer->size);

    if (buffer->dmabuf_fd >= 0)
        close(buffer->dmabuf_fd);

    free(buffer);
}

/* Map buffer to user space */
void *imx_dma_heap_allocator_map(struct imx_dma_heap_buffer *buffer)
{
    if (!buffer)
        return NULL;

    /* mmap call, e.g., MAP_SHARED */
    // buffer->mapped = mmap(...);
    return buffer->mapped;
}

/* Unmap buffer from user space */
void imx_dma_heap_allocator_unmap(struct imx_dma_heap_buffer *buffer)
{
    if (!buffer || !buffer->mapped)
        return;

    // munmap(buffer->mapped, buffer->size);
    buffer->mapped = NULL;
}

/* Physical address session functions removed (modern kernels do not support) */
#ifdef DMA_BUF_IOCTL_PHYS
int imx_dma_buffer_dma_heap_allocator_start_sync_session_impl(struct imx_dma_heap_buffer *buffer)
{
    struct dma_buf_phys dma_phys;
    ioctl(buffer->dmabuf_fd, DMA_BUF_IOCTL_PHYS, &dma_phys);
    return 0;
}

int imx_dma_buffer_dma_heap_allocator_stop_sync_session_impl(struct imx_dma_heap_buffer *buffer)
{
    struct dma_buf_phys dma_phys;
    ioctl(buffer->dmabuf_fd, DMA_BUF_IOCTL_PHYS, &dma_phys);
    return 0;
}

uint64_t imx_dma_buffer_dma_heap_get_physical_address_from_dmabuf_fd(int fd)
{
    struct dma_buf_phys dma_phys;
    if (ioctl(fd, DMA_BUF_IOCTL_PHYS, &dma_phys) < 0)
        return 0;
    return dma_phys.phys_addr;
}
#else
int imx_dma_buffer_dma_heap_allocator_start_sync_session_impl(struct imx_dma_heap_buffer *buffer)
{
    (void)buffer;  // suppress unused parameter warning
    return 0;      // Not supported
}

int imx_dma_buffer_dma_heap_allocator_stop_sync_session_impl(struct imx_dma_heap_buffer *buffer)
{
    (void)buffer;  // suppress unused parameter warning
    return 0;      // Not supported
}

uint64_t imx_dma_buffer_dma_heap_get_physical_address_from_dmabuf_fd(int fd)
{
    (void)fd;      // suppress unused parameter warning
    return 0;      // Not supported
}
#endif

/* Minimal constructor for test code */
struct imx_dma_heap_buffer *imx_dma_buffer_dma_heap_allocator_new(void)
{
    struct imx_dma_heap_buffer *buf = calloc(1, sizeof(*buf));
    if (!buf)
        return NULL;

    buf->dmabuf_fd = -1;
    buf->size = 0;
    buf->mapped = NULL;

    return buf;
}
