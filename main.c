#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

int directory_exists(const char *name) {
    DIR *dir = opendir(".");
    if (!dir) return 0;

    struct dirent *entry;
    int found = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, name) == 0) {
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
            char filepath[256];
            strcpy(filepath, name);
            size_t len = strlen(filepath);
            if (len > 0 && filepath[len - 1] != '/') {
                strcat(filepath, "/");
            }
            char filepath1[256];
            char filepath2[256];
            char filepath3[256];
            strcpy(filepath1, filepath);
            strcat(filepath1, "reports.dat");
            int fd = creat(filepath1, 0664);   // octal 0664 = rw-rw-r--
            if (fd != -1) {
                close(fd);
                printf("File 'reports.dat' created successfully.\n");
            } else {
                perror("Failed to create 'reports.dat'");
            }
            strcpy(filepath2, filepath);
            strcat(filepath2, "district.cfg");
            fd = creat(filepath2, 0640);   // octal 0640 = rw-r-----
            if (fd != -1) {
                close(fd);
                printf("File 'district.cfg' created successfully.\n");
            } else {
                perror("Failed to create 'district.cfg'");
            }
            strcpy(filepath3, filepath);
            strcat(filepath3, "logged_district");
            fd = creat(filepath3, 0644);   // octal 0644 = rw-r--r--
            if (fd != -1) {
                close(fd);
                printf("File 'logged_district' created successfully.\n");
            } else {
                perror("Failed to create 'logged_district'");
            }
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