#include <util/log.h>

#include <errno.h>
#include <stddef.h>
#include <sys/mman.h>


extern void *mmap_userspace_add(void *addr, size_t len, int prot);

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off) {
	log_debug("addr=%p, len=%d, prot=%d, flags=%d, fd=%d, off=%d", addr, len, prot, flags, fd, off);
	if (len == 0) {
		SET_ERRNO(EINVAL);
		return MAP_FAILED;
	}

	if (flags & MAP_ANONYMOUS) {
		void *ptr = mmap_userspace_add(addr, len, prot);

		if (ptr == NULL) {
			errno = EPERM;
			return MAP_FAILED;
		}

		return ptr;
	} else {
		SET_ERRNO(ENOTSUP);
		return MAP_FAILED;
	}
}
