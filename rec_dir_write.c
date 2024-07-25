#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>
#include <dirent.h>

char *get_acl_path(char *path) {
    char *last_slash = strrchr(path, '/');
            
    size_t path_len = strlen(path);

    int dest_len = path_len + strlen(last_slash + 1) + 5;

    char *dest = (char *) malloc(dest_len);

    strcpy(dest, path);
    strcat(dest, "/");
    strcat(dest, last_slash + 1);
    strcat(dest, "_acl");

    return dest;
}

void folder_traverse(char *dir_path, char *acl_path){
    printf("dir_path: %s\n", dir_path);
    // check if acl_path exists
    struct stat sb;
    if (stat(acl_path, &sb) == -1) {
        perror("stat");
        return;
    }

    // check if 'w' permission is in acl_header

    // if 'w' permission is in acl_header, then loop over all files and subdirectories in dir_path

    // for each file or subdirectory, check if it is a directory or a file

    // if it is a directory, then recursively call folder_traverse on it

    // if it is a file, then call setacl on it

    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        char *name = entry->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }

        char *path = malloc(strlen(dir_path) + strlen(name) + 2);
        sprintf(path, "%s/%s", dir_path, name);

        struct stat sb;
        if (stat(path, &sb) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(sb.st_mode)) {
            char *new_acl_path = get_acl_path(path);
            printf("dir: %s\n", path);
            folder_traverse(path, new_acl_path);
        } else if (S_ISREG(sb.st_mode)) {
            printf("file: %s\n", path);
            // setacl(path, acl_path);
        } else {
            printf("Unknown file type\n");
        }

        free(path);
    }
}

int main(int argc, char *argv[]) {
    char *dir_path = "/home/kali/Desktop/simple_slash/rdir";

    char *path = "/home/kali/Desktop/simple_slash/rdir/rdir_acl";

    folder_traverse(dir_path, path);

    return 0;
}