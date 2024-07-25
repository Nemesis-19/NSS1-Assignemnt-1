#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

struct file_data {
    uid_t owner;
    unsigned int dac_len;
    char **dac;
    unsigned int acl_len;
    char **acl;
    unsigned int data_len;
    char *data;
};

void write_file(const char *filename, struct file_data *file) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    // Write owner
    fwrite(&file->owner, sizeof(file->owner), 1, fp);

    // Write DAC length and DAC strings
    fwrite(&file->dac_len, sizeof(file->dac_len), 1, fp);
    for (unsigned int i = 0; i < file->dac_len; i++) {
        unsigned int dac_string_len = strlen(file->dac[i]) + 1;
        fwrite(&dac_string_len, sizeof(dac_string_len), 1, fp);
        fwrite(file->dac[i], dac_string_len, 1, fp);
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
    
    fclose(fp);
}

int main(int argc, char *argv[]) {
    struct file_data *file = malloc(sizeof(struct file_data));

    uid_t owner_id = getuid();
    file->owner = owner_id;

    file->dac_len = 2;
    file->dac = malloc(sizeof(char *) * file->dac_len);
    file->dac[0] = strdup("1003:rwx");
    file->dac[1] = strdup("other:rx");

    file->acl_len = 2;
    file->acl = malloc(sizeof(char *) * file->acl_len);
    file->acl[0] = strdup("1004:r");
    file->acl[1] = strdup("1005:rw");

    // owner_str = owner + ":rwx"
    // char *owner_str = malloc(sizeof(char) * 10);
    // sprintf(owner_str, "%d:rwx", owner);
    // file->owner = strdup(owner_str);

    // char uid_str[10];
    // sprintf(uid_str, "%d", owner);

    // // owner_str = uid_str + ":rwx"
    // char *owner_str = malloc(sizeof(char) * 10);
    // strcpy(owner_str, uid_str);
    // strcat(owner_str, ":rwx");
    // strcpy(file->owner, owner_str);

    // // other_str = "1000:r"
    // strcpy(file->other, "other:rx");

    char *data = "Hello, My name is Alice. I am a student at the University of Michigan.";
    file->data_len = strlen(data) + 1;
    file->data = strdup(data);

    write_file("alice_data", file);

    for (unsigned int i = 0; i < file->dac_len; i++) {
        free(file->dac[i]);
    }
    for (unsigned int i = 0; i < file->acl_len; i++) {
        free(file->acl[i]);
    }
    free(file->dac);
    free(file->acl);
    free(file->data);
    free(file);

    return 0;
}