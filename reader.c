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

struct file_data {
    uid_t owner;
    unsigned int dac_len;
    char **dac;
    unsigned int acl_len;
    char **acl;
    unsigned int data_len;
    char *data;
};

char *get_acl_path(char *path) {
    size_t path_len = strlen(path);

    int dest_len = path_len + 4;

    char *dest = (char *) malloc(dest_len);

    strcpy(dest, path);
    strcat(dest, "_acl");

    return dest;
}

int main() {
    // read /home/kali/Desktop/simple_slash/fget_acl

    FILE *fp = fopen("/home/kali/Desktop/simple_slash/fget_ac", "rb");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    return 0;
}