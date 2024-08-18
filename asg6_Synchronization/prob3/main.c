#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#define FILE_NAME "output.txt"
#define BUFFER_SIZE 128

static int counter = 0;
int get_last_number(const char *filename) 
{
    FILE *file;
    char buffer[BUFFER_SIZE];
    char *last_line = NULL;
    int last_number = -1;

    // Open the file
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    // Read through the file to find the last line
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Store the last line read
        if(strlen(buffer) == 0) {
            return 0;
        }
        last_line = strdup(buffer);
    }

    fclose(file);

    if (last_line != NULL) {
        // Extract the number from the last line
        char *token = strrchr(last_line, ':'); // Find the last ':'
        if (token != NULL) {
            last_number = atoi(token + 1); // Convert the number after ':'
        }
        free(last_line); // Free the duplicated line
    }

    return last_number;
}
int write_file(const char* filename, int pid, int number)
{
    FILE *file;

    // Open the file
    file = fopen(filename, "a");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    // Write the line to the file
    if (fprintf(file, "%d: %d\n", pid, number) < 0) {
        perror("Error writing to file");
        fclose(file);
        return -1;
    }
    fclose(file);
    return 1;
}

static void signal_handle(int num)
{
    return;
}

int main()
{
    pid_t pid;
    int number;

    if(signal(SIGUSR1, signal_handle) == SIG_ERR) {
        printf("Error to handle signal\n");
        return -1;
    }
    pid = fork();
    
    if(pid == -1) {
        printf("Failed to fork\n");
        return -1;
    } else if(pid > 0) {
        printf("Parent: %d\nChild: %d\n", getpid(), pid);
        // parent process
        for (int i = 0; i < 10; i++) {
            counter++;
            number = get_last_number(FILE_NAME);
            if(number == -1) {
                write_file(FILE_NAME, getpid(), counter);
            } else {
                number++;
                write_file(FILE_NAME, getpid(), number);
            }
            kill(pid, SIGUSR1);
            pause();
        }
    } else {
        // chhild process
        for (int i = 0; i < 10; i++) {
            pause();
            number = get_last_number(FILE_NAME);
            number++;
            write_file(FILE_NAME, getpid(), number);
            kill(getppid(), SIGUSR1);
        }
        
    }
    
    return 0;
}