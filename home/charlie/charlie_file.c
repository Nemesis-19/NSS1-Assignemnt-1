#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct acl_entry {
    // here user id will come
    char *user;
    char *permission;
};

struct file_data {
    unsigned int acl_len;
    struct acl_entry *acl;
    unsigned int data_len;
    char *data;
};

int main(int argc, char *argv[]) {
    struct file_data *file = malloc(sizeof(struct file_data));
    file->acl_len = 3;
    file->acl = malloc(sizeof(struct acl_entry) * file->acl_len);

    file->acl[0].user = "1005";
    file->acl[0].permission = "rwx";
    file->acl[1].user = "1004";
    file->acl[1].permission = "r";
    file->acl[2].user = "1003";
    file->acl[2].permission = "rx";

    // data is "Hello, My name is Charlie. My best friend is Ron."

    file->data_len = 50;
    file->data = malloc(file->data_len);
    strcpy(file->data, "Hello, My name is Charlie. My best friend is Ron.");

    printf("file->acl[0].user: %s\n", file->acl[0].user);
    printf("file->acl[0].permission: %s\n", file->acl[0].permission);
    printf("file->acl[1].user: %s\n", file->acl[1].user);
    printf("file->acl[1].permission: %s\n", file->acl[1].permission);
    printf("file->acl[2].user: %s\n", file->acl[2].user);
    printf("file->acl[2].permission: %s\n", file->acl[2].permission);
    printf("file->data: %s\n", file->data);

    FILE *fp = fopen("charlie_data", "wb");
    fwrite(file, sizeof(struct file_data), 1, fp);
    fclose(fp);

    free(file->acl);
    free(file->data);
    free(file);

    return 0;
}