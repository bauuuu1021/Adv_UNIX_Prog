#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXARGS 31
#define MAX_PATH_SIZE 256
#define ERR -1
#define DEBUG 0
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
    fprintf(tty_fd, "[sandbox] " __VA_ARGS__);                                 \
  } while (0)

#define old_func(type, name, ...) static type (*old_##name)(__VA_ARGS__) = NULL

#define handle_old_func(name)                                                  \
  if (old_##name == NULL) {                                                    \
    void *handle = dlopen("libc.so.6", RTLD_LAZY);                             \
    if (handle)                                                                \
      old_##name = dlsym(handle, #name);                                       \
  }                                                                            \
  if (old_##name == NULL) {                                                    \
    debug("old function not found\n");                                         \
  }

#define check_range(func_name, target_path, ret_err)                           \
  do {                                                                         \
    char s[MAX_PATH_SIZE] = {0};                                               \
    realpath(target_path, s);                                                  \
    if (!strlen(cwd))                                                          \
      break;                                                                   \
    int len = (strlen(cwd) < strlen(s)) ? strlen(cwd) : strlen(s);             \
    if (strncmp(cwd, s, len) ||                                                \
        (strlen(s) == 1 && strlen(cwd) != strlen(s))) {                        \
      info("%s : access to %s is not allowed\n", func_name, target_path);      \
      return ret_err;                                                          \
    }                                                                          \
  } while (0)

FILE *tty_fd;
char cwd[MAX_PATH_SIZE] = {0};
struct stat64 *_avoid_warning;

old_func(int, __xstat, int ver, const char *path, struct stat *stat_buf);
old_func(int, __xstat64, int ver, const char *path, struct stat64 *stat_buf);
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
old_func(int, symlinkat, const char *target, int newdirfd,
         const char *linkpath);
old_func(int, unlink, const char *__name);

int __xstat(int ver, const char *path, struct stat *stat_buf) {
  handle_old_func(__xstat);
  check_range(__func__, path, ERR);
  return old___xstat(ver, path, stat_buf);
}

int __xstat64(int ver, const char *path, struct stat64 *stat_buf) {
  handle_old_func(__xstat64);
  check_range(__func__, path, ERR);
  return old___xstat64(ver, path, stat_buf);
}

int chdir(const char *path) {
  handle_old_func(chdir);
  check_range(__func__, path, ERR);
  return old_chdir(path);
}

int chmod(const char *__file, mode_t __mode) {
  handle_old_func(chmod);
  check_range(__func__, __file, ERR);
  return old_chmod(__file, __mode);
}

int chown(const char *__file, uid_t __owner, gid_t __group) {
  handle_old_func(chown);
  check_range(__func__, __file, ERR);
  return old_chown(__file, __owner, __group);
}

int creat(const char *__file, mode_t __mode) {
  handle_old_func(creat);
  check_range(__func__, __file, ERR);
  return old_creat(__file, __mode);
}

FILE *fopen(const char *__restrict __filename, const char *__restrict __modes) {
  handle_old_func(fopen);
  check_range(__func__, __filename, NULL);
  return old_fopen(__filename, __modes);
}

int link(const char *__from, const char *__to) {
  handle_old_func(link);
  check_range(__func__, __from, ERR);
  check_range(__func__, __to, ERR);
  return old_link(__from, __to);
}

int mkdir(const char *__path, __mode_t __mode) {
  handle_old_func(mkdir);
  check_range(__func__, __path, ERR);
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
  check_range(__func__, __path, ERR);
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
  check_range(__func__, __path, ERR);
  return old_openat(__fd, __path, __oflag, mode);
}

DIR *opendir(const char *__nam) {
  handle_old_func(opendir);
  char s[MAX_PATH_SIZE] = {0};
  realpath(__nam, s);
  int len = strlen(cwd);
  if (strncmp(cwd, s, len) || (strlen(s) == 1 && strlen(cwd) != strlen(s))) {
    info("%s : access to %s is not allowed\n", "opendir", __nam);
    return NULL;
  }
  return old_opendir(__nam);
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
  handle_old_func(readlink);
  check_range(__func__, pathname, ERR);
  return old_readlink(pathname, buf, bufsiz);
}

int remove(const char *__filename) {
  handle_old_func(remove);
  check_range(__func__, __filename, ERR);
  return old_remove(__filename);
}

int rename(const char *__old, const char *__new) {
  handle_old_func(rename);
  check_range(__func__, __old, ERR);
  check_range(__func__, __new, ERR);
  return old_rename(__old, __new);
}

int rmdir(const char *__path) {
  handle_old_func(rmdir);
  check_range(__func__, __path, ERR);
  return old_rmdir(__path);
}

int symlink(const char *target, const char *linkpath) {
  handle_old_func(symlink);
  check_range(__func__, target, ERR);
  check_range(__func__, linkpath, ERR);
  return old_symlink(target, linkpath);
}

int symlinkat(const char *target, int newdirfd, const char *linkpath) {
  handle_old_func(symlink);
  check_range(__func__, target, ERR);
  check_range(__func__, linkpath, ERR);
  return old_symlink(target, linkpath);
}

int unlink(const char *__name) {
  handle_old_func(unlink);
  check_range(__func__, __name, ERR);
  return old_unlink(__name);
}

int execl(const char *__path, const char *__arg, ...) {
  info("reject : %s(%s)\n", __func__, __path);
  return ERR;
}

int execle(const char *__path, const char *__arg, ...) {
  info("reject : %s(%s)\n", __func__, __path);
  return ERR;
}

int execlp(const char *__file, const char *__arg, ...) {
  info("reject : %s(%s)\n", __func__, __file);
  return ERR;
}

int execv(const char *__path, char *const __argv[]) {
  info("reject : %s(%s)\n", __func__, __path);
  return ERR;
}

int execve(const char *__path, char *const __argv[], char *const __envp[]) {
  info("reject : %s(%s)\n", __func__, __path);
  return ERR;
}

int execvp(const char *__path, char *const __argv[]) {
  info("reject : %s(%s)\n", __func__, __path);
  return ERR;
}

int system(const char *command) {
  info("reject : %s(%s)\n", __func__, command);
  return ERR;
}

__attribute__((constructor)) static void init() {
  tty_fd = fopen("/dev/tty", "w");
  strcpy(cwd, getenv("sandbox_basedir"));
  debug("constructor : cwd is %s\n", cwd);
}

__attribute__((destructor)) static void end() { fclose(tty_fd); }