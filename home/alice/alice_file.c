#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct file_data {
    unsigned int acl_len;
    unsigned char **acl;
    unsigned int data_len;
    unsigned char *data;
};

int main(int argc, char *argv[]) {
    struct file_data *file = malloc(sizeof(struct file_data));
    file->acl_len = 3;
    file->acl = malloc(sizeof(unsigned char *) * file->acl_len);
    // ACL for alice: 1004:r, 1003:rwx
    file->acl[0] = malloc(7);
    strcpy(file->acl[0], "1004:r");
    file->acl[1] = malloc(9);
    strcpy(file->acl[1], "1003:rwx");
    file->acl[2] = malloc(7);
    strcpy(file->acl[2], "1005:w");

    // file->acl[0].user = "1004";
    // file->acl[0].permission = "r";
    // file->acl[1].user = "1003";
    // file->acl[1].permission = "rwx";

    // data is "Hello, My name is Alice. I am a student at the University of Michigan."
    char *data = "Hello, My name is Alice. I am a student at the University of Michigan.";
    file->data_len = strlen(data) + 1;
    file->data = malloc(file->data_len);
    strcpy(file->data, data);

    // printf("file->acl[0].user: %s\n", file->acl[0].user);
    // printf("file->acl[0].permission: %s\n", file->acl[0].permission);
    // printf("file->acl[1].user: %s\n", file->acl[1].user);
    // printf("file->acl[1].permission: %s\n", file->acl[1].permission);
    // printf("file->data_len: %d\n", file->data_len);
    // printf("file->data: %s\n", file->data);

    printf("file->acl[0]: %s\n", file->acl[0]);
    printf("file->acl[1]: %s\n", file->acl[1]);
    printf("file->acl[2]: %s\n", file->acl[2]);
    printf("file->data_len: %d\n", file->data_len);
    printf("file->data: %s\n", file->data);

    FILE *fp = fopen("alice_data", "wb");
    // writing to file, but size will be size of struct + size of acl + size of data
    fwrite(file, sizeof(struct file_data), 1, fp);
    fclose(fp);

    free(file->acl);
    free(file->data);
    free(file);

    return 0;
}