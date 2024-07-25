#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

struct file_data {
    unsigned int acl_len;
    char **acl;
    unsigned int data_len;
    char *data;
    uid_t owner;
};

void write_file(const char *filename, struct file_data *file) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    // Write ACL length and ACL strings
    fwrite(&file->acl_len, sizeof(file->acl_len), 1, fp);
    for (unsigned int i = 0; i < file->acl_len; i++) {
        unsigned int acl_string_len = strlen(file->acl[i]) + 1;
        fwrite(&acl_string_len, sizeof(acl_string_len), 1, fp);
        fwrite(file->acl[i], acl_string_len, 1, fp);
    }

    // Write data length and data string
    fwrite(&file->data_len, sizeof(file->data_len), 1, fp);
    fwrite(file->data, file->data_len, 1, fp);

    // Write owner
    fwrite(&file->owner, sizeof(file->owner), 1, fp);
    
    fclose(fp);
}

int main(int argc, char *argv[]) {
    struct file_data *file = malloc(sizeof(struct file_data));
    file->acl_len = 4;
    file->acl = malloc(sizeof(char *) * file->acl_len);
    file->acl[0] = strdup("1004:r");
    file->acl[1] = strdup("1003:rwx");
    file->acl[2] = strdup("1005:r");

    file->owner = getuid();

    char *data = "These are acls for rdir directory. Please do not delete them unless you know what you are doing.";
    file->data_len = strlen(data) + 1;
    file->data = strdup(data);

    write_file("/home/kali/Desktop/simple_slash/rdir/rdir_acl", file);

    for (unsigned int i = 0; i < file->acl_len; i++) {
        free(file->acl[i]);
    }
    free(file->acl);
    free(file->data);
    free(file);

    return 0;
}