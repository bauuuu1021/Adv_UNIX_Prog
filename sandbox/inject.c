#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

#define DEBUG 1 
#if DEBUG 
#define debug(...) do { printf("[debug] "); printf(__VA_ARGS__); } while (0)
#else
#define debug(...) do {  } while (0)
#endif

#define info(...)  do { printf("[sandbox] "); printf(__VA_ARGS__); } while (0)

#define old_func(type, name, ...) \
	static type (*old_##name)(__VA_ARGS__) = NULL;

#define handle_old_func(name) \
	debug("call __"#name"()\n");\
	if (old_##name == NULL) {\
		void *handle = dlopen("libc.so.6", RTLD_LAZY);\
		if (handle)\
			old_##name = dlsym(handle, #name);\
	}\
	if (old_##name == NULL) {\
		debug("old function not found\n");\
	}

old_func(uid_t, getuid, void)
old_func(int, __xstat, int ver, const char * path, struct stat * stat_buf)
old_func(int, __lxstat, int ver, const char * path, struct stat * stat_buf)
old_func(int, __fxstat, int vers, int fd, struct stat *buf)

uid_t getuid(void) {
	handle_old_func(getuid);
	return old_getuid();
}

int __xstat(int ver, const char * path, struct stat * stat_buf) {
	handle_old_func(__xstat);
	return old___xstat(ver, path, stat_buf);
}


int __lxstat(int ver, const char * path, struct stat * stat_buf) {
	handle_old_func(__lxstat);
	return old___lxstat(ver, path, stat_buf);
}

int __fxstat(int vers, int fd, struct stat *buf) {
	handle_old_func(__fxstat);
	return old___fxstat(vers, fd, buf);
}
