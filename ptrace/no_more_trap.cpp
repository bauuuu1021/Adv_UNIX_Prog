#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <string.h>
#include <iostream>

#include "ptools.h"

using namespace std;

void errquit(const char *msg) {
	perror(msg);
	exit(-1);
}

void dump_code(long addr, long code) {
	fprintf(stderr, "## %lx: code = %02x %02x %02x %02x %02x %02x %02x %02x\n",
		addr,
		((unsigned char *) (&code))[0],
		((unsigned char *) (&code))[1],
		((unsigned char *) (&code))[2],
		((unsigned char *) (&code))[3],
		((unsigned char *) (&code))[4],
		((unsigned char *) (&code))[5],
		((unsigned char *) (&code))[6],
		((unsigned char *) (&code))[7]);
}

int main(int argc, char *argv[]) {
	pid_t child;
	long offset = 0xe4; // the default value we reverse engineered
	if(argc > 1) {
		offset = strtol(argv[1], NULL, 0);
	}
	//fprintf(stderr, "## offset = %ld (0x%lx)\n", offset, offset);
	if((child = fork()) < 0) errquit("fork");
	if(child == 0) {
		if(ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) errquit("ptrace");
		execlp("./demo/no_more_traps", "./demo/no_more_traps", NULL);
		errquit("execvp");
	} else {
		int status;
		unsigned long long code;
		map<range_t, map_entry_t> vmmap;
		map<range_t, map_entry_t>::iterator vi;

		if(waitpid(child, &status, 0) < 0) errquit("waitpid");
		assert(WIFSTOPPED(status));
		ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL);

		/* continue the execution */
		struct user_regs_struct regs;
		FILE *fp = fopen("no_more_traps.txt", "r");
		char buf[2] = {0};

		int i = 0;
		ptrace(PTRACE_CONT, child, 0, 0);
		while (waitpid(child, &status, 0) > 0) {
			if(!WIFSTOPPED(status)) continue;
			if(ptrace(PTRACE_GETREGS, child, 0, &regs) != 0)
				errquit("ptrace(GETREGS)");
			if (!fread(buf, 1, 2, fp)) break;
			//std::cout << i << std::endl;
			i++;
			code = ptrace(PTRACE_PEEKTEXT, child, regs.rip-1, 0);
			
			if (code & 0x00000000000000ff != 0xcc) continue;
			ptrace(PTRACE_POKETEXT, child, regs.rip-1, (code & 0xffffffffffffff00) | strtol(buf, NULL, 16));
			//std::cout << (code & 0xffffffffffffff00) | atol(buf);
			
			regs.rip = regs.rip-1;
			if(ptrace(PTRACE_SETREGS, child, 0, &regs) != 0) errquit("ptrace(SETREGS)");
			ptrace(PTRACE_CONT, child, 0, 0);

			memset(buf, 0, sizeof(buf));
		} 


	}
	return 0;
}

