#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

void sig_handler1(int signum) {
    printf("Im signal handler1\n");
    exit(EXIT_SUCCESS);
}

int main() {
  	sigset_t new_set, old_set;
	if (signal(SIGINT, sig_handler1) == SIG_ERR) {
                fprintf(stderr, "Cannot handle SIGINT\n");
                exit(EXIT_FAILURE);
    }
    sigemptyset(&new_set);
	sigemptyset(&old_set);
    sigaddset(&new_set, SIGINT);

    if (sigprocmask(SIG_SETMASK, &new_set, &old_set) == 0) {
        if (sigismember(&new_set, SIGINT) == 1 ) {
            printf("Block SIGINT\n");
        } else if (sigismember(&new_set, SIGINT) == 0) {
            printf("Unblock SIGINT\n");
        }
    }

	while (1);		
	return 0;
}