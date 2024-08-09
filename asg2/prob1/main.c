#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

void show_dir_content(char *path, int level)
{
    DIR *dir;
    struct dirent *dirEntry;   

    dir = opendir(path);
    if(dir == NULL) {
        printf("Failed to open directory\n");
        exit(0);
    }
    while ((dirEntry = readdir(dir)) != NULL) {
        switch (dirEntry->d_type) {
            case DT_REG:
                for(int i = 0; i < level; i++) {
                    printf("\t");
                }
                printf("%s: regular file\n", dirEntry->d_name);
                break;
            case DT_BLK:
                for(int i = 0; i < level; i++) {
                    printf("\t");
                }
                printf("%s: block device\n", dirEntry->d_name);
                break;
            case DT_CHR:
                for(int i = 0; i < level; i++) {
                    printf("\t");
                }
                printf("%s: character device\n", dirEntry->d_name);
                break;
            case DT_FIFO:
                for(int i = 0; i < level; i++) {
                    printf("\t");
                }
                printf("%s: named pipe\n", dirEntry->d_name);
                break;
            case DT_SOCK:
                for(int i = 0; i < level; i++) {
                    printf("\t");
                }
                printf("%s: UNIX domain socket\n", dirEntry->d_name);
                break;
            case DT_DIR:
                if (strcmp(dirEntry->d_name, "..") != 0 && strcmp(dirEntry->d_name, ".") != 0) {
                    for(int i = 0; i < level; i++) {
                        printf("\t");
                    }
                    printf("%s: directory\n", dirEntry->d_name);
                    char d_path[255];
                    sprintf(d_path, "%s/%s", path, dirEntry->d_name);
                    show_dir_content(d_path, level + 1); // recall with the new path
                }
                break;
            default:
                printf("%s: couldn't be determined\n", dirEntry->d_name);
                break;
        }
    }
    closedir(dir);
}
int main(int argc, char **argv)
{       
    if (argc != 2) {
      printf ("Need a name or a path to directory\n");
      exit(0);
    }
    show_dir_content(argv[1], 0);
    return 0;
}