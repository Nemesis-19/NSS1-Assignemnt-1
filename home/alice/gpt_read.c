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

void read_file(const char *filename, struct file_data *file) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    // Read owner
    fread(&file->owner, sizeof(file->owner), 1, fp);

    // Read DAC length
    fread(&file->dac_len, sizeof(file->dac_len), 1, fp);

    // Allocate memory for DAC strings
    file->dac = malloc(sizeof(char *) * file->dac_len);

    // Read DAC strings
    for (unsigned int i = 0; i < file->dac_len; i++) {
        unsigned int dac_string_len;
        fread(&dac_string_len, sizeof(dac_string_len), 1, fp);
        file->dac[i] = malloc(dac_string_len);
        fread(file->dac[i], dac_string_len, 1, fp);
    }

    // Read ACL length
    fread(&file->acl_len, sizeof(file->acl_len), 1, fp);

    // Allocate memory for ACL strings
    file->acl = malloc(sizeof(char *) * file->acl_len);

    // Read ACL strings
    for (unsigned int i = 0; i < file->acl_len; i++) {
        unsigned int acl_string_len;
        fread(&acl_string_len, sizeof(acl_string_len), 1, fp);
        file->acl[i] = malloc(acl_string_len);
        fread(file->acl[i], acl_string_len, 1, fp);
    }

    // Read data length
    fread(&file->data_len, sizeof(file->data_len), 1, fp);

    // Allocate memory for data
    file->data = malloc(file->data_len);

    // Read data
    fread(file->data, file->data_len, 1, fp);

    fclose(fp);
}

int main(int argc, char *argv[]) {
    struct file_data *file = malloc(sizeof(struct file_data));

    read_file("alice_data", file);

    printf("Owner: %d\n", file->owner);

    printf("\nDAC:\n");
    for (unsigned int i = 0; i < file->dac_len; i++) {
        printf("%s\n", file->dac[i]);
    }

    printf("\nACL:\n");
    for (unsigned int i = 0; i < file->acl_len; i++) {
        printf("%s\n", file->acl[i]);
    }

    printf("\nData:\n%s\n", file->data);

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
