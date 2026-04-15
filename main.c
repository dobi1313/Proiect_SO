#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

int directory_exists(const char *name) {
    DIR *dir = opendir(".");  // open current directory
    if (!dir) return 0;

    struct dirent *entry;
    int found = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, name) == 0) {
            // Check if it's a directory
            struct stat st;
            if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
                found = 1;
                break;
            }
        }
    }
    closedir(dir);
    return found;
}


void add_directory(const char *name) {
    if (directory_exists(name)) {
        printf("Directory '%s' already exists.\n", name);
    } else {
        if (mkdir(name, 0750) == 0) {
            printf("Directory '%s' created successfully.\n", name);
        } else {
            perror("mkdir failed");
        }
    }
}




int main(int argc, char *argv[]){
    if(argc < 6){
        printf("Too Few Arguments\n");
        return 1;
    }
    char role[10], user[20];
    if(strcmp(argv[1], "--role") != 0){
        printf("Invalid Role Argument\n");
        return 1;
    }
    strcpy(role, argv[2]);
    if(strcmp(argv[3], "--user") != 0){
        printf("Invalid User Argument\n");
        return 1;
    }
    strcpy(user, argv[4]);
    printf("Role: %s\n", role);
    printf("User: %s\n", user);
    add_directory(user);
    
    // ./city_manager --role manager --user alice --add downtown
    return 0;
}