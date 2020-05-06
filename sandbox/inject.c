#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXARGS 31
#define DEBUG 1
#if DEBUG
#define debug(...)                                                             \
  do {                                                                         \
    printf("[debug] " __VA_ARGS__);                                            \
  } while (0)
#else
#define debug(...)                                                             \
  do {                                                                         \
  } while (0)
#endif

#define info(...)                                                              \
  do {                                                                         \
    printf("[sandbox] " __VA_ARGS__);                                          \
  } while (0)

#define old_func(type, name, ...) static type (*old_##name)(__VA_ARGS__) = NULL

#define handle_old_func(name)                                                  \
  debug("call __" #name "()\n");                                               \
  if (old_##name == NULL) {                                                    \
    void *handle = dlopen("libc.so.6", RTLD_LAZY);                             \
    if (handle)                                                                \
      old_##name = dlsym(handle, #name);                                       \
  }                                                                            \
  if (old_##name == NULL) {                                                    \
    debug("old function not found\n");                                         \
  }

old_func(uid_t, getuid, void);
old_func(int, __xstat, int ver, const char *path, struct stat *stat_buf);
old_func(int, __lxstat, int ver, const char *path, struct stat *stat_buf);
old_func(int, __fxstat, int vers, int fd, struct stat *buf);
old_func(int, chdir, const char *path);
old_func(int, chmod, const char *__file, mode_t __mode);
old_func(int, chown, const char *__file, uid_t __owner, gid_t __group);
old_func(int, creat, const char *__file, mode_t __mode);
old_func(FILE *, fopen, const char *__restrict __filename,
         const char *__restrict __modes);
old_func(int, link, const char *__from, const char *__to);
old_func(int, mkdir, const char *__path, __mode_t __mode);
old_func(int, open, const char *__path, int __oflag, ...);
old_func(int, openat, int __fd, const char *__path, int __oflag, ...);
old_func(DIR *, opendir, const char *__nam);
old_func(ssize_t, readlink, const char *pathname, char *buf, size_t bufsiz);
old_func(int, remove, const char *__filename);
old_func(int, rename, const char *__old, const char *__new);
old_func(int, rmdir, const char *__path);
old_func(int, symlink, const char *target, const char *linkpath);
old_func(int, unlink, const char *__name);

uid_t getuid(void) {
  handle_old_func(getuid);
  return old_getuid();
}

int __xstat(int ver, const char *path, struct stat *stat_buf) {
  handle_old_func(__xstat);
  return old___xstat(ver, path, stat_buf);
}

int __lxstat(int ver, const char *path, struct stat *stat_buf) {
  handle_old_func(__lxstat);
  return old___lxstat(ver, path, stat_buf);
}

int __fxstat(int vers, int fd, struct stat *buf) {
  handle_old_func(__fxstat);
  return old___fxstat(vers, fd, buf);
}

int chdir(const char *path) {
  handle_old_func(chdir);
  return old_chdir(path);
}

int chmod(const char *__file, mode_t __mode) {
  handle_old_func(chmod);
  return old_chmod(__file, __mode);
}

int chown(const char *__file, uid_t __owner, gid_t __group) {
  handle_old_func(chown);
  return old_chown(__file, __owner, __group);
}

int creat(const char *__file, mode_t __mode) {
  handle_old_func(creat);
  return old_creat(__file, __mode);
}

FILE *fopen(const char *__restrict __filename, const char *__restrict __modes) {
  handle_old_func(fopen);
  return old_fopen(__filename, __modes);
}

int link(const char *__from, const char *__to) {
  handle_old_func(link);
  return old_link(__from, __to);
}

int mkdir(const char *__path, __mode_t __mode) {
  handle_old_func(mkdir);
  return old_mkdir(__path, __mode);
}

int open(const char *__path, int __oflag, ...) {
  handle_old_func(open);
  int mode = 0;
  if (__OPEN_NEEDS_MODE(__oflag)) {

    va_list arg;
    va_start(arg, __oflag);
    mode = va_arg(arg, int);
    va_end(arg);
  }
  return old_open(__path, __oflag, mode);
}

int openat(int __fd, const char *__path, int __oflag, ...) {
  handle_old_func(openat);
  int mode = 0;
  if (__OPEN_NEEDS_MODE(__oflag)) {

    va_list arg;
    va_start(arg, __oflag);
    mode = va_arg(arg, int);
    va_end(arg);
  }
  return old_openat(__fd, __path, __oflag, mode);
}

DIR *opendir(const char *__nam) {
  handle_old_func(opendir);
  return old_opendir(__nam);
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
  handle_old_func(readlink);
  return old_readlink(pathname, buf, bufsiz);
}

int remove(const char *__filename) {
  handle_old_func(remove);
  return old_remove(__filename);
}

int rename(const char *__old, const char *__new) {
  handle_old_func(rename);
  return old_rename(__old, __new);
}

int rmdir(const char *__path) {
  handle_old_func(rmdir);
  return old_rmdir(__path);
}

int symlink(const char *target, const char *linkpath) {
  handle_old_func(symlink);
  return old_symlink(target, linkpath);
}

int unlink(const char *__name) {
  handle_old_func(unlink);
  return old_unlink(__name);
}

int execl(const char *__path, const char *__arg, ...) {
  info("reject %s\n", __func__);
  return -1;
}

int execle(const char *__path, const char *__arg, ...) {
  info("reject %s\n", __func__);
  return -1;
}

int execlp(const char *__file, const char *__arg, ...) {
  info("reject %s\n", __func__);
  return -1;
}

int execv(const char *__path, char *const __argv[]) {
  info("reject %s\n", __func__);
  return -1;
}

int execve(const char *__path, char *const __argv[], char *const __envp[]) {
  info("reject %s\n", __func__);
  return -1;
}

int system(const char *command) {
  info("reject %s\n", __func__);
  return -1;
}
