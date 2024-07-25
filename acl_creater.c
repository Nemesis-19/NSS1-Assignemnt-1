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

char *get_acl_path(char *path) {
    size_t path_len = strlen(path);

    int dest_len = path_len + 4;

    char *dest = (char *) malloc(dest_len);

    strcpy(dest, path);
    strcat(dest, "_acl");

    return dest;
}

int main(int argc, char *argv[]) {
    char path_array[6][20] = {"setfacl", "getfacl", "fgetc", "fputc", "create_dir", "change_dir"};

    for(int i=0; i<6; i++){

        struct file_data *file = malloc(sizeof(struct file_data));

        uid_t owner_id = getuid();
        file->owner = owner_id;

        file->dac_len = 2;
        file->dac = malloc(sizeof(char *) * file->dac_len);
        file->dac[0] = strdup("1001:rwx");
        file->dac[1] = strdup("other:rx");

        file->acl_len = 3;
        file->acl = malloc(sizeof(char *) * file->acl_len);
        file->acl[0] = strdup("1004:rwx");
        file->acl[1] = strdup("1005:rwx");
        file->acl[2] = strdup("1003:rwx");

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

        char *data_for_acl = malloc(strlen(path_array[i]) + 78);
        strcpy(data_for_acl, "This is the ACL file for program ");
        strcat(data_for_acl, path_array[i]);
        strcat(data_for_acl, ", modify only if you know what you are doing");

        file->data_len = strlen(data_for_acl) + 1;
        file->data = strdup(data_for_acl);

        char *path_acl = get_acl_path(path_array[i]);

        write_file(path_acl, file);

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

        free(data_for_acl);
        free(path_acl);
    }

    return 0;
}