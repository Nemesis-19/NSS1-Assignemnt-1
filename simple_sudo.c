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
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    // check if the name after last slash belongs to {"fget", "fput", "getfacl", "change_dir", "create_dir", "setfacl"}
    char *last_slash = strrchr(argv[1], '/');
    char *last_name = last_slash + 1;
    char *allowed_names[] = {"fgetc", "fputc", "getfacl", "change_dir", "create_dir", "setfacl"};
    int allowed = 0;
    for (int i = 0; i < 6; i++) {
        if (strcmp(last_name, allowed_names[i]) == 0) {
            allowed = 1;
            break;
        }
    }
    if (!allowed) {
        printf("Error: %s is not allowed\n", last_name);
        return 1;
    }

    // add _acl to the end of the file name
    char *acl_path = get_acl_path(argv[1]);

    printf("ACL path: %s\n", acl_path);

    // read file
    struct file_data *data = malloc(sizeof(struct file_data));

    read_file(acl_path, data);

    uid_t uid = getuid();

    // check if the user has permission to read the file, that is there is an acl entry for the user with w in permission
    // first convert the uid to a string
    char uid_str[10];
    sprintf(uid_str, "%d", uid);
    // then iterate through the acl entries and check if there is an entry for the user with w in permission
    printf("Invoking UID: %s\n", uid_str);

    int is_owner = 0;
    int is_acl = 0;
    int is_other = 0;
    int has_permission = 0;

    // check if user is the owner of the file and owner has x permission
    if (data->owner == uid) {
        is_owner = 1;
        if (strchr(data->dac[0], 'x') != NULL) {
            has_permission = 1;
        }
    }

    if (is_owner == 1 && has_permission == 0){
        printf("You do not have permission to execute this file\n");
        return 1;
    }

    // loop through the acl entries and check if the user id matches
    for (unsigned int i = 0; i < data->acl_len; i++) {
        if (strstr(data->acl[i], uid_str) != NULL) {
            is_acl = 1;
            if (strchr(data->acl[i], 'x') != NULL) {
                has_permission = 1;
                break;
            }
        }
    }

    if (is_acl == 1 && has_permission == 0){
        printf("You do not have permission to execute this file\n");
        return 1;
    }

    // check for other acl entry
    if (has_permission == 0 && strchr(data->dac[1], 'x') != NULL) {
        is_other = 1;
        has_permission = 1;
    }

    if (has_permission == 0) {
        printf("You do not have permission to execute this file\n");
        return 1;
    }

    printf("You have permission to execute this file\n");

    // setuid to the owner of argv[0] and execute argv[1] with argv[0] as argument

    // get the owner of argv[0]
    struct stat sb;
    if (stat(argv[0], &sb) == -1) {
        perror("stat");
        return 1;
    }

    // setuid to the owner of argv[0]
    if (setuid(sb.st_uid) == -1) {
        perror("seteuid");
        return 1;
    }

    // execute argv[0] with argv[1] as argument
    char *args[] = {argv[0], NULL};
    execv(argv[1], args);

    // change back to the original user
    if (setuid(uid) == -1) {
        perror("seteuid");
        return 1;
    }

    // free memory
    for (unsigned int i = 0; i < data->dac_len; i++) {
        free(data->dac[i]);
    }
    free(data->dac);
    for (unsigned int i = 0; i < data->acl_len; i++) {
        free(data->acl[i]);
    }
    free(data->acl);
    free(data->data);
    free(data);

    free(acl_path);

    return 0;
}