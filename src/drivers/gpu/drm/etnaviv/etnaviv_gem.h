/**
 * @file
 *
 * @date Jan 18, 2018
 * @author Anton Bondarev
 */
#ifndef SRC_DRIVERS_GPU_DRM_ETNAVIV_ETNAVIV_GEM_H_
#define SRC_DRIVERS_GPU_DRM_ETNAVIV_ETNAVIV_GEM_H_

#include <stdint.h>
#include <pthread.h>

#include <linux/list.h>

#include <embox_drm/drm_priv.h>
#include <embox_drm/drm_gem.h>

#include "etnaviv_compat.h"
#include "etnaviv_cmdbuf.h"

struct etnaviv_gpu;
struct etnaviv_gem_ops;
struct etnaviv_gem_object;

struct etnaviv_gem_object {
	struct drm_gem_object base;
	const struct etnaviv_gem_ops *ops;
	pthread_mutex_t lock;

	struct list_head gem_node;
	struct etnaviv_gpu *gpu;     /* non-null if active */

	int flags;
};

struct vm_area_struct;
struct etnaviv_gem_ops {
	int (*get_pages)(struct etnaviv_gem_object *);
	void (*release)(struct etnaviv_gem_object *);
	void *(*vmap)(struct etnaviv_gem_object *);
	int (*mmap)(struct etnaviv_gem_object *, struct vm_area_struct *);
};


extern int etnaviv_gem_mmap_offset(struct drm_gem_object *obj, uint64_t *offset);

static inline
struct etnaviv_gem_object *to_etnaviv_bo(struct drm_gem_object *obj)
{
	//return container_of(obj, struct etnaviv_gem_object, base);
	return member_cast_out(obj, struct etnaviv_gem_object, base);
}


extern int etnaviv_gem_new_handle(struct drm_device *dev, struct drm_file *file,
		uint32_t size, uint32_t flags, uint32_t *handle);

extern struct drm_gem_object *etnaviv_gem_new(struct drm_device *dev,
		uint32_t size, uint32_t flags);

extern int etnaviv_ioctl_gem_info(struct drm_device *dev, void *data,
		struct drm_file *file);

extern int etnaviv_ioctl_gem_submit(struct drm_device *dev, void *data,
		struct drm_file *file);

struct etnaviv_gem_submit_bo {
u32 flags;
struct etnaviv_gem_object *obj;
struct etnaviv_vram_mapping *mapping;
};


struct etnaviv_gem_submit {
	struct drm_device *dev;
	struct etnaviv_gpu *gpu;
	struct ww_acquire_ctx ticket;
	struct dma_fence *fence;
	unsigned int nr_bos;
	struct etnaviv_gem_submit_bo bos[0];
	u32 flags;
};

struct etnaviv_vram_mapping {
	struct list_head obj_node;
	struct list_head scan_node;
	struct list_head mmu_node;
	struct etnaviv_gem_object *object;
	struct etnaviv_iommu *mmu;
	struct drm_mm_node vram_node;
	unsigned int use;
	u32 iova;
};

extern  int etnaviv_gpu_submit(struct etnaviv_gpu *gpu,
	struct etnaviv_gem_submit *submit, struct etnaviv_cmdbuf *cmdbuf);

#endif /* SRC_DRIVERS_GPU_DRM_ETNAVIV_ETNAVIV_GEM_H_ */
