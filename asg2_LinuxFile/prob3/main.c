#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>

void create_links(const char *directory, const char *filename) {
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);

    // Create hard link
    char hardlink[1024];
    snprintf(hardlink, sizeof(hardlink), "%s_hardlink", filename);
    if (link(filepath, hardlink) == -1) {
        printf("Failed to create hard link");
    } else {
        printf("Hard link created: %s\n", hardlink);
    }

    // Create symbolic link
    char softlink[1024];
    sprintf(softlink, "%s_softlink", filename);
    if (symlink(filepath, softlink) == -1) {
        printf("Failed to create soft link");
    } else {
        printf("Soft link created: %s\n", softlink);
    }
}

int main(int argc, char **argv)
{
    if(argc != 3) {
        printf("Form to use this program: ./exe path_to_directory file_name \n");
        return 0;
    }
    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        printf("Can not open the directory\n");
        return 0;
    }

    struct dirent *entry;
    int file_found = false;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, argv[2]) == 0) {
            file_found = true;
            create_links(argv[1], entry->d_name);
            break;
        }
    }

    if (!file_found) {
        printf("File '%s' not found in directory '%s'.\n", argv[2], argv[1]);
    }
    closedir(dir);
    return 0;

}