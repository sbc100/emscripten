#include <semaphore.h>
#include <sys/mman.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>
#include "lock.h"
#include "fork_impl.h"
#include "emscripten/console.h"

#define malloc __libc_malloc
#define calloc __libc_calloc
#define realloc undef
#define free undef

static struct {
	ino_t ino;
	sem_t *sem;
	int refcnt;
} *semtab;
static volatile int lock[1];
volatile int *const __sem_open_lockptr = lock;

#define FLAGS (O_RDWR|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK)

sem_t *sem_open(const char *name, int flags, ...)
{
	va_list ap;
	mode_t mode;
	unsigned value;
	int fd, i, e, slot, first=1, cnt, cs;
	sem_t newsem;
	void *map;
	char tmp[64];
	struct timespec ts;
	struct stat st;
	char buf[NAME_MAX+10];

  _emscripten_errf("sem_open %s", name);
	if (!(name = __shm_mapname(name, buf)))
		return SEM_FAILED;
  _emscripten_errf("__shm_mapname %s", name);

	LOCK(lock);
	/* Allocate table if we don't have one yet */
	if (!semtab && !(semtab = calloc(sizeof *semtab, SEM_NSEMS_MAX))) {
		UNLOCK(lock);
		return SEM_FAILED;
	}

	/* Reserve a slot in case this semaphore is not mapped yet;
	 * this is necessary because there is no way to handle
	 * failures after creation of the file. */
	slot = -1;
	for (cnt=i=0; i<SEM_NSEMS_MAX; i++) {
		cnt += semtab[i].refcnt;
		if (!semtab[i].sem && slot < 0) slot = i;
	}
	/* Avoid possibility of overflow later */
	if (cnt == INT_MAX || slot < 0) {
		errno = EMFILE;
		UNLOCK(lock);
		return SEM_FAILED;
	}
	/* Dummy pointer to make a reservation */
	semtab[slot].sem = (sem_t *)-1;
	UNLOCK(lock);

	flags &= (O_CREAT|O_EXCL);

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

	/* Early failure check for exclusive open; otherwise the case
	 * where the semaphore already exists is expensive. */
	if (flags == (O_CREAT|O_EXCL) && access(name, F_OK) == 0) {
		errno = EEXIST;
		goto fail;
	}

	for (;;) {
		/* If exclusive mode is not requested, try opening an
		 * existing file first and fall back to creation. */
		if (flags != (O_CREAT|O_EXCL)) {
			fd = open(name, FLAGS);
			if (fd >= 0) {
        _emscripten_errf("open done: %d", fd);
				if (fstat(fd, &st) < 0 ||
				    (map = mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
					close(fd);
					goto fail;
				}
				close(fd);
				break;
			}
			if (errno != ENOENT) {
        _emscripten_errf("failed to open");
				goto fail;
      }
		}
		if (!(flags & O_CREAT))  {
        _emscripten_errf("O_CREAT not passed");
			goto fail;
    }
		if (first) {
			first = 0;
			va_start(ap, flags);
			mode = va_arg(ap, mode_t) & 0666;
			value = va_arg(ap, unsigned);
			va_end(ap);
			if (value > SEM_VALUE_MAX) {
        _emscripten_errf("SEM_VALUE_MAX error");
				errno = EINVAL;
				goto fail;
			}
			sem_init(&newsem, 1, value);
		}
    _emscripten_errf("sem_open fd = %d", fd);
		/* Create a temp file with the new semaphore contents
		 * and attempt to atomically link it as the new name */
		clock_gettime(CLOCK_REALTIME, &ts);
		snprintf(tmp, sizeof(tmp), "/dev/shm/tmp-%d", (int)ts.tv_nsec);
		fd = open(tmp, O_CREAT|O_EXCL|FLAGS, mode);
		if (fd < 0) {
      _emscripten_errf("error opening %s", tmp);
			if (errno == EEXIST) continue;
			goto fail;
		}
    _emscripten_errf("done open %s %d", tmp, fd);
		if (write(fd, &newsem, sizeof newsem) != sizeof newsem || fstat(fd, &st) < 0 ||
		    (map = mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
			close(fd);
			unlink(tmp);
			goto fail;
		}
    _emscripten_errf("done write");
		close(fd);
    _emscripten_errf("linking %s %s", tmp, name);
		e = link(tmp, name) ? errno : 0;
		unlink(tmp);
		if (!e) break;
		munmap(map, sizeof(sem_t));
		/* Failure is only fatal when doing an exclusive open;
		 * otherwise, next iteration will try to open the
		 * existing file. */
		if (e != EEXIST || flags == (O_CREAT|O_EXCL))
      _emscripten_errf("yyyyy %s", strerror(e));
			goto fail;
	}

  _emscripten_errf("xxxxx");

	/* See if the newly mapped semaphore is already mapped. If
	 * so, unmap the new mapping and use the existing one. Otherwise,
	 * add it to the table of mapped semaphores. */
	LOCK(lock);
	for (i=0; i<SEM_NSEMS_MAX && semtab[i].ino != st.st_ino; i++);
	if (i<SEM_NSEMS_MAX) {
		munmap(map, sizeof(sem_t));
		semtab[slot].sem = 0;
		slot = i;
		map = semtab[i].sem;
	}
	semtab[slot].refcnt++;
	semtab[slot].sem = map;
	semtab[slot].ino = st.st_ino;
	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
	return map;

fail:
	pthread_setcancelstate(cs, 0);
	LOCK(lock);
	semtab[slot].sem = 0;
	UNLOCK(lock);
	return SEM_FAILED;
}

int sem_close(sem_t *sem)
{
	int i;
	LOCK(lock);
	for (i=0; i<SEM_NSEMS_MAX && semtab[i].sem != sem; i++);
	if (--semtab[i].refcnt) {
		UNLOCK(lock);
		return 0;
	}
	semtab[i].sem = 0;
	semtab[i].ino = 0;
	UNLOCK(lock);
	munmap(sem, sizeof *sem);
	return 0;
}
