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

// function to recursively get the acl of a directory
void folder_traverse(char *dir_path, char *acl_path, struct file_data *data) {
    printf("Folder Travesre for dir_path: %s\n", dir_path);
    // check if acl_path exists
    struct stat sb;
    if (stat(acl_path, &sb) == -1) {
        // create the acl file
        printf("Creating ACL file for %s\n", dir_path);
        FILE *fp = fopen(acl_path, "wb");
        if (fp == NULL) {
            perror("Error Creating ACL file");
            return;
        }

        // close the file
        fclose(fp);
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

        printf("Inside folder_traverse: %s\n", name);

        // generate the path of the file or directory
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
            folder_traverse(path, new_acl_path, data);
        } else if (S_ISREG(sb.st_mode)) {
            printf("file: %s\n", path);
            // setacl(path, acl_path);
            // check if we are writing to the acl file of the directory
            if (strcmp(acl_path, path) == 0) {
                // using seteuid to change the invoking user to owner of the file
                // extract the owner from data->owner
                // using seteuid to change the invoking user to owner of the file

                // write the updated acl entry to the file
                FILE *fp = fopen(path, "wb");
                if (fp == NULL) {
                    perror("Error opening file");
                    return;
                }

                // Write owner
                fwrite(&data->owner, sizeof(data->owner), 1, fp);

                // Write DAC length
                fwrite(&data->dac_len, sizeof(data->dac_len), 1, fp);

                // Write DAC strings
                for (unsigned int i = 0; i < data->dac_len; i++) {
                    unsigned int dac_string_len = strlen(data->dac[i]) + 1;
                    fwrite(&dac_string_len, sizeof(dac_string_len), 1, fp);
                    fwrite(data->dac[i], dac_string_len, 1, fp);
                }

                // Write ACL length
                fwrite(&data->acl_len, sizeof(data->acl_len), 1, fp);

                // Write ACL strings
                for (unsigned int i = 0; i < data->acl_len; i++) {
                    unsigned int acl_string_len = strlen(data->acl[i]) + 1;
                    fwrite(&acl_string_len, sizeof(acl_string_len), 1, fp);
                    fwrite(data->acl[i], acl_string_len, 1, fp);
                }

                // get the name of the directory from dir_path
                char *dir_name = basename(dir_path);

                // data string is "This is the ACL file for directory <dir_name>, modify only if you know what you are doing"
                char *data_for_acl = malloc(strlen(dir_name) + 80);
                strcpy(data_for_acl, "This is the ACL file for directory ");
                strcat(data_for_acl, dir_name);
                strcat(data_for_acl, ", modify only if you know what you are doing");
                
                // Write data length of data_for_acl
                unsigned int data_len = strlen(data_for_acl) + 1;
                fwrite(&data->data_len, sizeof(data->data_len), 1, fp);

                // Write data
                fwrite(data_for_acl, data->data_len, 1, fp);

                free(data_for_acl);

                fclose(fp);

            } 
            // else {
            //     // read from the file update only the dac_len, dac, acl_len, acl

            //     struct file_data *prev_data = malloc(sizeof(struct file_data));

            //     read_file(path, prev_data);

            //     printf("For file path %s: ", path);


            // }
            
        } else {
            printf("Unknown file type\n");
        }

        free(path);
    }
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

    // get the id of the user you wish to change the acl entry of
    printf("Enter the UID of the user you wish to change the ACL entry of [input 'OTHER' for other or Specific UID for user]: ");
    char uid_to_change[10];
    scanf("%s", uid_to_change);
    
    int change_other = 0;
    int change_owner = 0;
    int change_specific = 0;
    int create_new = 0;

    // string to store the user's acl entry if it exists
    char *user_acl = NULL;

    // check if the user wants to change the other dac entry
    if (strcmp(uid_to_change, "OTHER") == 0) {
        change_other = 1;
        user_acl = data->dac[1];
    }

    // check if the uid exists
    if (change_other == 0) {
        // convert uid_to_change to uid_t
        unsigned long uid_ulong = strtoul(uid_to_change, NULL, 10);
        // Check for conversion errors
        if (uid_ulong == 0 && uid_str[0] != '0') {
            perror("Conversion error");
            return 1;
        }
        // Assign the converted value to uid_t
        uid_t uid_change = (uid_t)uid_ulong;
        struct passwd *pwd = getpwuid(uid_change);
        if (pwd == NULL) {
            printf("User does not exist\n");
            return 1;
        }
    }

    // check if uid_to_change is the owner of the file
    // convert data->owner to string
    char owner_str[10];
    sprintf(owner_str, "%d", data->owner);
    if (strcmp(uid_to_change, owner_str) == 0) {
        change_owner = 1;
        user_acl = data->dac[0];
    }

    // check if the user wants to change the acl entry of a specific user
    int acl_index = -1;
    for (unsigned int i = 0; i < data->acl_len; i++) {
        if (strstr(data->acl[i], uid_to_change) != NULL) {
            change_specific = 1;
            user_acl = data->acl[i];
            acl_index = i;
            break;
        }
    }

    // check if the user wants to create a new acl entry
    if (change_other == 0 && change_owner == 0 && change_specific == 0) {
        create_new = 1;
    }

    // ask the user for the new acl entry
    printf("Enter the new ACL entry [choose from rwx, rw, rx, wx, r, w, x]: ");
    char new_acl[10];
    scanf("%s", new_acl);

    // check if the new acl entry is valid
    if (strcmp(new_acl, "rwx") != 0 && strcmp(new_acl, "rw") != 0 && strcmp(new_acl, "rx") != 0 && strcmp(new_acl, "wx") != 0 && strcmp(new_acl, "r") != 0 && strcmp(new_acl, "w") != 0 && strcmp(new_acl, "x") != 0) {
        printf("Invalid ACL entry\n");
        return 1;
    }
    
    if (create_new == 0){
        if (change_other == 1){
            // update the other dac entry
            char *new_other_acl = malloc(strlen("other:") + strlen(new_acl) + 2);
            strcpy(new_other_acl, "other");
            strcat(new_other_acl, ":");
            strcat(new_other_acl, new_acl);

            // update the dac entry
            user_acl = new_other_acl;

            // update the dac array
            data->dac[1] = user_acl;
        }
        else if (change_owner == 1){
            // update the owner dac entry
            char *new_owner_acl = malloc(strlen(owner_str) + strlen(new_acl) + 2);
            strcpy(new_owner_acl, owner_str);
            strcat(new_owner_acl, ":");
            strcat(new_owner_acl, new_acl);

            // update the dac entry
            user_acl = new_owner_acl;

            // update the dac array
            data->dac[0] = user_acl;
        }
        else if (change_specific == 1){
            // update the user's acl entry
            char *new_user_acl = malloc(strlen(uid_to_change) + strlen(new_acl) + 2);
            strcpy(new_user_acl, uid_to_change);
            strcat(new_user_acl, ":");
            strcat(new_user_acl, new_acl);

            // update the acl entry
            user_acl = new_user_acl;

            // update the acl array
            data->acl[acl_index] = user_acl;
        }
    }
    else {
        // create the user's acl entry
        char *new_user_acl = malloc(strlen(uid_to_change) + strlen(new_acl) + 2);
        strcpy(new_user_acl, uid_to_change);
        strcat(new_user_acl, ":");
        strcat(new_user_acl, new_acl);

        // update the acl entry
        user_acl = new_user_acl;

        // update the acl length
        data->acl_len += 1;

        // update the acl array
        data->acl = realloc(data->acl, sizeof(char *) * data->acl_len);

        // add the new acl entry to the acl array
        data->acl[data->acl_len - 1] = user_acl;
    }

    // print the updated acl entry
    printf("Your updated ACL entry: %s\n", user_acl);

    if (is_dir) {
        printf("Updating ACL of directory %s\n", dir_path);
        printf("Starting recursive traversal of directory %s\n", dir_path);

        // using seteuid to change the invoking user to owner of the file
        if (seteuid(data->owner) == -1) {
            perror("seteuid");
            return 1;
        }

        folder_traverse(dir_path, path, data);

        // change the invoking user back to the original user
        if (seteuid(uid) == -1) {
            perror("seteuid");
            return 1;
        }

        // free the file data
        for (unsigned int i = 0; i < data->acl_len; i++) {
            free(data->acl[i]);
        }
        for (unsigned int i = 0; i < data->dac_len; i++) {
            free(data->dac[i]);
        }
        free(data->acl);
        free(data->dac);
        free(data->data);
        free(data);

        return 0;
    }

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