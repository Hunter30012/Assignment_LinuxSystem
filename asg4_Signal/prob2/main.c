#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

static int flag_interrupt = 0;
static void signal_handler(int signum)
{
    if(signum == SIGINT) {
        printf("Do not stop the process running\n");
        flag_interrupt = 1;
    }
}
int main(int argc, char **argv)
{
    if(signal(SIGINT, signal_handler) == SIG_ERR) {
        printf("Failed to regis signal handler\n");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        if(flag_interrupt) {
            flag_interrupt = 0;
            printf("Continue running process\n");
        }
    }
    return 0;
}