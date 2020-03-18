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
  DIR *proc_dir, *current_dir;
  struct dirent *proc_ptr, *current_ptr;

  if (!(proc_dir = opendir("/proc"))) {
    printf("[find_pid] No Directory %s\n", "/proc");
    return -1;
  }

  while ((proc_ptr = readdir(proc_dir))) {
    /* Skip invalid process directories */
    if ((!strcmp(proc_ptr->d_name, ".")) || (!strcmp(proc_ptr->d_name, "..")))
      continue;

    char current_path[BUF_SIZE];
    snprintf(current_path, sizeof(current_path), "/proc/%s/fd",
             proc_ptr->d_name);
    if (!(current_dir = opendir(current_path))) {
      // printf("[find_pid] No Directory %s\n", current_path);
      continue;
    }
    // printf("%s\n", proc_ptr->d_name);

    while ((current_ptr = readdir(current_dir))) {
      if ((!strcmp(current_ptr->d_name, ".")) ||
          (!strcmp(current_ptr->d_name, "..")))
        continue;

      char buf[BUF_SIZE], path[BUF_SIZE], sock_buf[BUF_SIZE];
      memset(buf, 0, sizeof(buf));
      memset(path, 0, sizeof(path));
      memset(sock_buf, 0, sizeof(sock_buf));
      snprintf(path, sizeof(path), "%s/%s", current_path, current_ptr->d_name);
      // printf("%s\n", path);
      readlink(path, buf, sizeof(buf));
      snprintf(sock_buf, sizeof(sock_buf), "socket:[%d]", inode);
      if (!strcmp(buf, sock_buf)) {
        printf("---%s---\n", path);
        closedir(current_dir);
        closedir(proc_dir);

        return atoi(proc_ptr->d_name);
      }
    }

    closedir(current_dir);
  }

  closedir(proc_dir);
}

void tcp_conn() {
  int i;
  for (i=0; i<2000; i++)
  printf("pid %d\n", find_pid(25574));
  /*
  char buf[BUF_SIZE];
  memset(buf, 0, sizeof(buf));
  if(readlink("/proc/12324/fd/14", buf, sizeof(buf)) == -1) {
    printf("%s\n", strerror(errno));
  }
  printf("%s\n", buf);
  */
}

int main(int argc, char **argv) {

  // parse_arg(argc, argv);
  tcp_conn();
}
