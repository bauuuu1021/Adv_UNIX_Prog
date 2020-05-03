#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <stdio.h>

static uid_t (*old_getuid)(void) = NULL;

uid_t getuid(void) {
	if(old_getuid == NULL) {
		void *handle = dlopen("libc.so.6", RTLD_LAZY);
		if(handle != NULL) {
			old_getuid = dlsym(handle, "getuid");
		}
	}
	fprintf(stderr, "injected getuid, always return 0 (%s)\n", __FILE__);
	if(old_getuid != NULL) {
		fprintf(stderr, "real uid = %d\n", old_getuid());
	}
	return 0;
}

int __xstat(int ver, const char * path, struct stat * stat_buf) {
	fprintf(stderr, "injected stattttttttttt, always return 0 (%s)\n", __FILE__);
	return 123;	
}

int __lxstat(int ver, const char * path, struct stat * stat_buf) {
	fprintf(stderr, "injected lllllllllllllstat, always return 0 (%s)\n", __FILE__);
	return 0;
}
