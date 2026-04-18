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
            strcpy(filepath1, filepath);
            strcat(filepath1, "reports.dat");
            FILE *fp = fopen(filepath1, "wb");
            if (fp != NULL) {
                fclose(fp);
                if (chmod(filepath1, 0664) == -1) {
                    perror("chmod failed for reports.dat");
                } else {
                    printf("File 'reports.dat' created and permissions set to 0664.\n");
                }
            } else {
                perror("fopen failed for reports.dat");
            }

            
            char filepath2[256];
            strcpy(filepath2, filepath);
            strcat(filepath2, "district.cfg");
            fp = fopen(filepath2, "w");   // text mode is fine
            if (fp != NULL) {
                fclose(fp);
                if (chmod(filepath2, 0640) == -1) {
                    perror("chmod failed for district.cfg");
                } else {
                    printf("File 'district.cfg' created and permissions set to 0640.\n");
                }
            } else {
                perror("fopen failed for district.cfg");
            }

            
            char filepath3[256];
            strcpy(filepath3, filepath);
            strcat(filepath3, "logged_district");
            fp = fopen(filepath3, "w");
            if (fp != NULL) {
                fclose(fp);
                if (chmod(filepath3, 0644) == -1) {
                    perror("chmod failed for logged_district");
                } else {
                    printf("File 'logged_district' created and permissions set to 0644.\n");
                }
            } else {
                perror("fopen failed for logged_district");
            }

        } else {
            perror("mkdir failed");
        }
    }
}

#include <sys/stat.h>
#include <stdio.h>

void print_permissions(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("stat failed");
        return;
    }

    
    if (S_ISDIR(st.st_mode))      putchar('d');
    else if (S_ISLNK(st.st_mode)) putchar('l');
    else                          putchar('-');

    // Owner permissions
    putchar((st.st_mode & S_IRUSR) ? 'r' : '-');
    putchar((st.st_mode & S_IWUSR) ? 'w' : '-');
    putchar((st.st_mode & S_IXUSR) ? 'x' : '-');

    // Group permissions
    putchar((st.st_mode & S_IRGRP) ? 'r' : '-');
    putchar((st.st_mode & S_IWGRP) ? 'w' : '-');
    putchar((st.st_mode & S_IXGRP) ? 'x' : '-');

    // Others permissions
    putchar((st.st_mode & S_IROTH) ? 'r' : '-');
    putchar((st.st_mode & S_IWOTH) ? 'w' : '-');
    putchar((st.st_mode & S_IXOTH) ? 'x' : '-');

    printf(" %s\n", path);
}

void list_directory(const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    printf("\nContents of '%s':\n", dirpath);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);

        
        print_permissions(fullpath);
    }

    closedir(dir);
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
    if(strcmp(argv[5], "--add") == 0){
        char district[20];
        strcpy(district, argv[6]);
        printf("Adding district: %s\n", district);
        add_directory(district);
    }
    if(strcmp(argv[5], "--list") == 0){
        list_directory(argv[6]);
    }
    
    // ./city_manager --role manager --user alice --add downtown
    return 0;
}