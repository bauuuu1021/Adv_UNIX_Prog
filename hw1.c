#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR -1
#define BUF_SIZE 1024

#define DFLT 0x00
#define TCP 0x01
#define UDP 0x10
#define BOTH 0x11

#define skip_column(to_skip)                                                   \
  do {                                                                         \
    int column = to_skip;                                                      \
    while (column-- > 0) {                                                     \
      strtok(NULL, " ");                                                       \
    }                                                                          \
  } while (0)

/* Traverse all process directory and find pid with matched inode number(s) */
int find_pid(int inode) {
  DIR *proc_dir, *fd_dir;
  struct dirent *proc_content, *fd_content;

  if (!(proc_dir = opendir("/proc"))) {
    printf("[find_pid] No Directory %s\n", "/proc");
    return ERR;
  }

  while ((proc_content = readdir(proc_dir))) {
    if ((!strcmp(proc_content->d_name, ".")) ||
        (!strcmp(proc_content->d_name, "..")))
      continue;

    char path_to_fddir[BUF_SIZE];
    snprintf(path_to_fddir, sizeof(path_to_fddir), "/proc/%s/fd",
             proc_content->d_name);
    if (!(fd_dir = opendir(path_to_fddir))) {
      // printf("[find_pid] No Directory %s\n", path_to_fddir);
      continue;
    }
    // printf("%s\n", proc_content->d_name);

    while ((fd_content = readdir(fd_dir))) {
      if ((!strcmp(fd_content->d_name, ".")) ||
          (!strcmp(fd_content->d_name, "..")))
        continue;

      char symlink_dest[BUF_SIZE], path_to_symlink[BUF_SIZE],
          target_inode[BUF_SIZE];
      memset(symlink_dest, 0, sizeof(symlink_dest));
      memset(path_to_symlink, 0, sizeof(path_to_symlink));
      memset(target_inode, 0, sizeof(target_inode));

      snprintf(path_to_symlink, sizeof(path_to_symlink), "%s/%s", path_to_fddir,
               fd_content->d_name);
      // printf("%s\n", path_to_symlink);
      readlink(path_to_symlink, symlink_dest, sizeof(symlink_dest));
      snprintf(target_inode, sizeof(target_inode), "socket:[%d]", inode);
      if (!strcmp(symlink_dest, target_inode)) {
        // printf("---%s---\n", path_to_symlink);
        closedir(fd_dir);
        closedir(proc_dir);

        return atoi(proc_content->d_name);
      }
    }
    closedir(fd_dir);
  }
  closedir(proc_dir);
  return ERR; /* Search failed */
}

void list_connection(char *protocal, char *filter) {

  char network_file[BUF_SIZE];
  memset(network_file, 0, sizeof(network_file));
  snprintf(network_file, sizeof(network_file), "/proc/net/%s", protocal);
  FILE *net_fp = fopen(network_file, "r");
  char conn_info[BUF_SIZE], *local_addr, *foreign_addr, *inode;
  memset(conn_info, 0, sizeof(conn_info));

  /* Skip the title */
  fgets(conn_info, BUF_SIZE, net_fp);
  printf("Proto\tLocal Address\tForeign Address\tPID/Program name and "
         "arguments\n");

  while (fgets(conn_info, BUF_SIZE, net_fp)) {
    int pid;
    char cmd_path[BUF_SIZE];
    memset(cmd_path, 0, sizeof(cmd_path));
    FILE *read_cmd;
    char cmd_content[BUF_SIZE];
    memset(cmd_content, 0, sizeof(cmd_content));

    strtok(conn_info, " ");
    local_addr = strtok(NULL, " ");
    foreign_addr = strtok(NULL, " ");
    skip_column(6);
    inode = strtok(NULL, " ");

    pid = find_pid(atoi(inode));
    if (pid != ERR) {
      snprintf(cmd_path, sizeof(cmd_path), "/proc/%d/cmdline", pid);
      read_cmd = fopen(cmd_path, "r");
    }

    if (read_cmd) {
      fgets(cmd_content, BUF_SIZE, read_cmd);
      fclose(read_cmd);
    }

    /* string filter */
    if (filter && (!strstr(cmd_content, filter)))
      continue;
    printf("%s\t%s\t%s\t%6d / %s\n", protocal, local_addr, foreign_addr, pid,
           cmd_content);
  }
  fclose(net_fp);
}

void parse_arg(int argc, char **argv) {
  int c;
  int this_option_optind = optind ? optind : 1;
  int option_index = 0;
  int protocal = DFLT;
  char *filter = NULL;

  while (1) {
    static struct option long_options[] = {{"tcp", no_argument, 0, 't'},
                                           {"udp", no_argument, 0, 'u'},
                                           {0, 0, 0, 0}};

    c = getopt_long(argc, argv, "tu", long_options, &option_index);
    if (c == ERR)
      break;

    switch (c) {
    case 0:
      printf("option %s", long_options[option_index].name);
      if (optarg)
        printf(" with arg %s", optarg);
      printf("\n");
      break;

    case 't':
      protocal |= TCP;
      break;

    case 'u':
      protocal |= UDP;
      break;

    case '?':
      break;

    default:
      printf("?? getopt returned character code 0%o ??\n", c);
    }
  }

  if (optind < argc) {
    filter = argv[optind];
  }

  /* Dump connection information */
  if (protocal ^ 0x10) {
    list_connection("tcp", filter);
    list_connection("tcp6", filter);
  }
  if (protocal ^ 0x01) {
    list_connection("udp", filter);
    list_connection("udp6", filter);
  }
}

int main(int argc, char **argv) { parse_arg(argc, argv); }
