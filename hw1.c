#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define skip_column(to_skip)                                                   \
  do {                                                                         \
    int column = to_skip;                                                      \
    while (column-- > 0) {                                                     \
      strtok(NULL, " ");                                                       \
    }                                                                          \
  } while (0)

void parse_arg(int argc, char **argv) {
  int c;
  int this_option_optind = optind ? optind : 1;
  int option_index = 0;
  while (1) {
    static struct option long_options[] = {{"tcp", no_argument, 0, 't'},
                                           {"udp", no_argument, 0, 'u'},
                                           {0, 0, 0, 0}};

    c = getopt_long(argc, argv, "tu", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 0:
      printf("option %s", long_options[option_index].name);
      if (optarg)
        printf(" with arg %s", optarg);
      printf("\n");
      break;

    case 't':
      printf("tcp\n");
      break;

    case 'u':
      printf("udp\n");
      break;

    case '?':
      break;

    default:
      printf("?? getopt returned character code 0%o ??\n", c);
    }
  }

  if (optind < argc) {
    printf("non-option ARGV-elements: ");
    while (optind < argc)
      printf("%s ", argv[optind++]);
    printf("\n");
  }
}

/* Traverse all process directory and find pid with matched inode number(s) */
int find_pid(int inode) {
  DIR *proc_dir, *fd_dir;
  struct dirent *proc_content, *fd_content;

  if (!(proc_dir = opendir("/proc"))) {
    printf("[find_pid] No Directory %s\n", "/proc");
    return -1;
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
  return -1; /* Search failed */
}

void tcp_conn() {

  FILE *tcpv4 = fopen("/proc/net/tcp", "r");
  char conn_info[BUF_SIZE], *local_addr, *foreign_addr, *inode;
  memset(conn_info, 0, sizeof(conn_info));

  /* Skip the title */
  fgets(conn_info, BUF_SIZE, tcpv4);

  while (fgets(conn_info, BUF_SIZE, tcpv4)) {
    // printf("%s\n", conn_info);
    strtok(conn_info, " ");
    local_addr = strtok(NULL, " ");
    foreign_addr = strtok(NULL, " ");
    skip_column(6);
    inode = strtok(NULL, " ");

    int pid = find_pid(atoi(inode));
    char cmd_path[BUF_SIZE];
    memset(cmd_path, 0, sizeof(cmd_path));
    snprintf(cmd_path, sizeof(cmd_path), "/proc/%d/cmdline", pid);
    FILE *read_cmd = fopen(cmd_path, "r");
    char cmd_content[BUF_SIZE];
    memset(cmd_content, 0, sizeof(cmd_content));
    fgets(cmd_content, BUF_SIZE, read_cmd);

    printf("%s ; %s ; %d/%s\n", local_addr, foreign_addr, pid, cmd_content);
    fclose(read_cmd);
  }
  fclose(tcpv4);
}

int main(int argc, char **argv) {

  // parse_arg(argc, argv);
  tcp_conn();
}
