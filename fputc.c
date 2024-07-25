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

void write_file(const char *filename, struct file_data *file) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    // Write owner
    fwrite(&file->owner, sizeof(file->owner), 1, fp);

    // Write DAC length
    fwrite(&file->dac_len, sizeof(file->dac_len), 1, fp);

    // Write DAC strings
    for (unsigned int i = 0; i < file->dac_len; i++) {
        unsigned int dac_string_len = strlen(file->dac[i]) + 1;
        fwrite(&dac_string_len, sizeof(dac_string_len), 1, fp);
        fwrite(file->dac[i], dac_string_len, 1, fp);
    }

    // Write ACL length
    fwrite(&file->acl_len, sizeof(file->acl_len), 1, fp);

    // Write ACL strings
    for (unsigned int i = 0; i < file->acl_len; i++) {
        unsigned int acl_string_len = strlen(file->acl[i]) + 1;
        fwrite(&acl_string_len, sizeof(acl_string_len), 1, fp);
        fwrite(file->acl[i], acl_string_len, 1, fp);
    }

    // Write data length
    fwrite(&file->data_len, sizeof(file->data_len), 1, fp);

    // Write data
    fwrite(file->data, file->data_len, 1, fp);
    
    fclose(fp);
}

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

int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    uid_t uid = getuid();

    char *path = argv[0];

    int is_dir = 0;
    char *dir_path = NULL;

    // check if path is file or directory or neither
    struct stat path_info;

    if (stat(path, &path_info) == 0) {
        if (S_ISDIR(path_info.st_mode)) {
            printf("%s is a directory.\n", path);

            // change the path to the acl file
            dir_path = path;
            path = get_acl_path(path);

            is_dir = 1;

            printf("Updating PATH to %s\n", path);
        }
        else if (S_ISREG(path_info.st_mode)) {
            printf("%s is a regular file.\n", path);
        } 
        else {
            printf("%s is neither a directory nor a regular file.\n", path);
        }
    } 
    else {
        perror("stat");
        return 1;
    }

    int file_fd = open(path, O_RDONLY);
    if (file_fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // Read ACL and file data from the file 

    struct file_data *data = malloc(sizeof(struct file_data));

    read_file(path, data);

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

    // check if user is the owner of the file and owner has w permission
    if (data->owner == uid) {
        is_owner = 1;
        if (strchr(data->dac[0], 'w') != NULL) {
            has_permission = 1;
        }
    }

    if (is_owner == 1 && has_permission == 0){
        printf("You do not have permission to write to this file\n");
        return 1;
    }

    // loop through the acl entries and check if the user id matches
    for (unsigned int i = 0; i < data->acl_len; i++) {
        if (strstr(data->acl[i], uid_str) != NULL) {
            is_acl = 1;
            if (strchr(data->acl[i], 'w') != NULL) {
                has_permission = 1;
                break;
            }
        }
    }

    if (is_acl == 1 && has_permission == 0){
        printf("You do not have permission to write to this file\n");
        return 1;
    }

    // check for other acl entry
    if (has_permission == 0 && strchr(data->dac[1], 'w') != NULL) {
        is_other = 1;
        has_permission = 1;
    }

    if (has_permission == 0) {
        printf("You do not have permission to write to this file\n");
        return 1;
    }

    // get the new data which the user wants to write to the file
    char *new_data = NULL;
    size_t new_data_len = 0;
    printf("Enter the new data to be written to the file: ");
    getline(&new_data, &new_data_len, stdin);

    // update the data in the file_data struct
    data->data = new_data;
    data->data_len = strlen(new_data) + 1;

    // using seteuid to change the invoking user to owner of the file
    if (seteuid(data->owner) == -1) {
        perror("seteuid");
        return 1;
    }

    // write the updated acl entry to the file
    write_file(path, data);

    // change the invoking user back to the original user
    if (seteuid(uid) == -1) {
        perror("seteuid");
        return 1;
    }

    for (unsigned int i = 0; i < data->dac_len; i++) {
        free(data->dac[i]);
    }
    for (unsigned int i = 0; i < data->acl_len; i++) {
        free(data->acl[i]);
    }
    free(data->dac);
    free(data->acl);
    free(data->data);
    free(data);

    return 0;
}