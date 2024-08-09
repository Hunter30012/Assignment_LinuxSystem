#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    if(argc != 3) {
        printf("Require filename and string\n");
        return 0;
    }

    int fd = open(argv[1], O_CREAT | O_WRONLY, 0644);
    if(fd == -1) {
        printf("Failed to open file\n");
        return 0;
    }

    int ret = write(fd, argv[2], strlen(argv[2]));
    if(ret < 0) {
        printf("Could not write to file\n");
        close(fd);
        return 0;
    }
    close(fd);
    return 0;
}
