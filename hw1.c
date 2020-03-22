#include <arpa/inet.h>
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

#define IPv4 8
#define IPv6 32

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

int hex2int(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
}

char *readable_format(char *origin) {
  char ret[BUF_SIZE];
  memset(ret, 0, sizeof(ret));
  int split;

  if (!(origin[IPv4] - ':')) { /* IPv4 */
    split = IPv4;
    snprintf(ret, sizeof(ret), "%d.%d.%d.%d",
             hex2int(origin[6]) * 16 + hex2int(origin[7]),
             hex2int(origin[4]) * 16 + hex2int(origin[5]),
             hex2int(origin[2]) * 16 + hex2int(origin[3]),
             hex2int(origin[0]) * 16 + hex2int(origin[1]));
  } else if (!(origin[IPv6] - ':')) { /* IPv6 */
    split = IPv6;
    int word = 0;
    int index = 0;
    while (word < 4) {
      ret[index++] = origin[word * 8 + 6];
      ret[index++] = origin[word * 8 + 7];
      ret[index++] = origin[word * 8 + 4];
      ret[index++] = origin[word * 8 + 5];
      ret[index++] = ':';
      ret[index++] = origin[word * 8 + 2];
      ret[index++] = origin[word * 8 + 3];
      ret[index++] = origin[word * 8 + 0];
      ret[index++] = origin[word * 8 + 1];
      ret[index++] = ':';
      word++;
    }
    ret[index - 1] = '\0';

    unsigned char buf[sizeof(struct in6_addr)];
    char str[INET6_ADDRSTRLEN];
    const char *res = ret;
    inet_pton(AF_INET6, res, buf);
    inet_ntop(AF_INET6, buf, str, INET6_ADDRSTRLEN);
    memset(ret, 0, sizeof(ret));
    strcpy(ret, str);
  }

  /* port */
  char port_str[BUF_SIZE] = {'\0'};
  int port = hex2int(origin[split + 1]) * 4096 +
             hex2int(origin[split + 2]) * 256 +
             hex2int(origin[split + 3]) * 16 + hex2int(origin[split + 4]);
  if (port)
    snprintf(port_str, sizeof(port_str), ":%d", port);
  else
    snprintf(port_str, sizeof(port_str), ":*");
  strcat(ret, port_str);

  strcpy(origin, ret);
  return origin;
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
      if (read_cmd) {
        fgets(cmd_content, BUF_SIZE, read_cmd);
        fclose(read_cmd);
      }
    }

    /* string filter */
    if (filter && (!strstr(cmd_content, filter)))
      continue;

    char local_addr_bck[BUF_SIZE] = {'\0'};
    strcpy(local_addr_bck, local_addr);
    if (pid != ERR)
      printf("%s\t%-40s\t%-40s\t%6d | %s\n", protocal,
             readable_format(local_addr_bck), readable_format(foreign_addr),
             pid, cmd_content);
    else
      printf("%s\t%-40s\t%-40s\n", protocal, readable_format(local_addr_bck),
             readable_format(foreign_addr));
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
      return;

    default:
      printf("?? getopt returned character code 0%o ??\n", c);
    }
  }

  if (optind < argc) {
    filter = argv[optind];
  }

  /* Dump connection information */
  if (protocal != UDP) {
    printf("\nTCP\nProto\t%-40s\t%-40s\tPID/Program name and arguments\n",
           "Local Address", "Foreign Address");
    list_connection("tcp", filter);
    list_connection("tcp6", filter);
  }
  if (protocal != TCP) {
    printf("\nUDP\nProto\t%-40s\t%-40s\tPID/Program name and arguments\n",
           "Local Address", "Foreign Address");
    list_connection("udp", filter);
    list_connection("udp6", filter);
  }
}

int main(int argc, char **argv) { parse_arg(argc, argv); }