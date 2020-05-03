#include <iostream>
#include <unistd.h>
#include <string>

using namespace std;

#define DEBUG 1 
#if DEBUG 
#define debug(...) do { printf("[debug] "); printf(__VA_ARGS__); } while (0)
#else
#define debug(...) do {  } while (0)
#endif

#define info(...)  do { printf("[sandbox] "); printf(__VA_ARGS__); } while (0)

struct INJECT_OPT {
	string so_path;
	string base_dir;
	string cmd;
};

struct INJECT_OPT inject_opt;

void parse_arg(int ac, char **av) {
	int option;
	if (ac == 1) {
		info("no command given.\n");
		exit(1);
	}

	while ((option=getopt(ac, av, "d:p:"))!=-1) {
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
				printf("usage: ./sandbox [-p sopath] [-d basedir] [--] cmd [cmd args ...]\n");
				printf("-p: set the path to sandbox.so, default = ./sandbox.so\n");
				printf("-d: the base directory that is allowed to access, default = .\n");
				printf("--: separate the arguments for sandbox and for the executed command\n");
				exit(1);
		}
	}

	ac -= optind;
	av += optind;
	
	for(int i = 0; i < ac; i++) {
        inject_opt.cmd += av[i];
		inject_opt.cmd += " ";
	}

}

int main (int argc, char **argv) {

	inject_opt.so_path = "./sandbox.so";
	parse_arg(argc, argv);
	string cmd = "LD_PRELOAD=" + inject_opt.so_path +" "+inject_opt.cmd;

	debug("cmd is %s\n", cmd.c_str());	
	system(cmd.c_str());
}
