#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>

using namespace std;

#define MAX_PATH_SIZE 256
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
    fprintf(tty_fd, "[sandbox] " __VA_ARGS__);                                 \
  } while (0)

FILE *tty_fd;

struct INJECT_OPT {
  string so_path;
  string base_dir;
  string cmd;
};

struct INJECT_OPT inject_opt;

string get_real_path(string cwd, string filename) {

  string prefix, origin, postfix;
  prefix = origin = cwd + filename;
  postfix = "";
  char ret[MAX_PATH_SIZE] = {0};

  while (!realpath(prefix.c_str(), ret)) {
    auto split = prefix.find_last_of("/");
    postfix = origin.substr(split);
    prefix = origin.substr(0, split);
  }

  return string(ret) + postfix;
}

void parse_arg(int ac, char **av) {
  int option;
  if (ac == 1) {
    info("no command given.\n");
    exit(1);
  }

  while ((option = getopt(ac, av, "d:p:")) != -1) {
    switch (option) {
    case 'd':
      inject_opt.base_dir = optarg;
      debug("set base dir %s\n", (inject_opt.base_dir).c_str());
      break;
    case 'p':
      inject_opt.so_path = optarg;
      debug("use shared obj %s\n", (inject_opt.so_path).c_str());
      break;
    default:
      printf("usage: ./sandbox [-p sopath] [-d basedir] [--] cmd [cmd args "
             "...]\n");
      printf("-p: set the path to sandbox.so, default = ./sandbox.so\n");
      printf("-d: the base directory that is allowed to access, default = .\n");
      printf("--: separate the arguments for sandbox and for the executed "
             "command\n");
      exit(1);
    }
  }

  ac -= optind;
  av += optind;

  string cwd = string(getcwd(NULL, 0)) + "/";
  for (int i = 0; i < ac; i++) {
    // cmd `sh ...`
    if (!strncmp(av[i], "sh", 2)) {
      while (i < ac)
        inject_opt.cmd = inject_opt.cmd + av[i++] + " ";
      break;
    }
    // cases for `/bin/ls ...` or `ls ...` or options
    else if ((!strncmp(av[i], "/", 1)) || (!i && strncmp(av[i], ".", 1)) ||
             (!strncmp(av[i], "-", 1)))
      inject_opt.cmd += av[i];
    // cases that need to get abs. path
    else
      inject_opt.cmd += get_real_path(cwd, av[i]);
    inject_opt.cmd += " ";
  }
}

int main(int argc, char **argv) {
  tty_fd = fopen("/dev/tty", "w");

  inject_opt.base_dir = ".";
  inject_opt.so_path = "./sandbox.so";
  parse_arg(argc, argv);

  string base_dir =
      "cd " + string(realpath((inject_opt.base_dir).c_str(), NULL)) + ";";
  string cmd = base_dir + "LD_PRELOAD=" +
               string(realpath((inject_opt.so_path).c_str(), NULL)) + " " +
               inject_opt.cmd;

  debug("cmd is %s\n", cmd.c_str());
  system(cmd.c_str());

  fclose(tty_fd);
}
