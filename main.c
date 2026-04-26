#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    int id;
    float X;
    float Y;
    char name[20];
    char category[20];
    int severity;
    char description[256];
    char timestamp[26];
} Report;




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
        return;
    } else {
        if (mkdir(name, 0750) == 0) {
            char filepath[256];
            strcpy(filepath, name);
            size_t len = strlen(filepath);
            if (len > 0 && filepath[len - 1] != '/') {
                strcat(filepath, "/");
            }

            
            char filepath1[256];
            strcpy(filepath1, filepath);
            strcat(filepath1, "reports.dat");
            int fd = open(filepath1, O_WRONLY | O_CREAT | O_TRUNC, 0664);
            if (fd != -1) {
                close(fd);
                if (chmod(filepath1, 0664) == -1) {
                    perror("chmod failed for reports.dat");
                } 
            } else {
                perror("open failed for reports.dat");
            }

            
            char filepath2[256];
            strcpy(filepath2, filepath);
            strcat(filepath2, "district.cfg");
            fd = open(filepath2, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (fd != -1) {
                char buf[16];
                int len = snprintf(buf, sizeof(buf), "%d%d", 1, 0);
                write(fd, buf, len);
                close(fd);
                if (chmod(filepath2, 0640) == -1) {
                    perror("chmod failed for district.cfg");
                }
            } else {
                perror("open failed for district.cfg");
            }

            
            char filepath3[256];
            strcpy(filepath3, filepath);
            strcat(filepath3, "logged_district");
            fd = open(filepath3, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd != -1) {
                close(fd);
                if (chmod(filepath3, 0644) == -1) {
                    perror("chmod failed for logged_district");
                } 
            } else {
                perror("open failed for logged_district");
            }

        } else {
            perror("mkdir failed");
        }
    }
}
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
        strcpy(fullpath, dirpath);
        if (fullpath[strlen(fullpath) - 1] != '/') {
            strcat(fullpath, "/");
        }
        strcat(fullpath, entry->d_name);
        print_permissions(fullpath);
    }
    closedir(dir);
}
void update_config(const char *dirpath, int severity, int next_id) {
    char filepath[256];
    strcpy(filepath, dirpath);
    strcat(filepath, "/district.cfg");
    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
    if (fd == -1) return;
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "%d%d", severity, next_id);
    write(fd, buf, len);
    close(fd);
}
int read_config(const char *dirpath,int *severity, int *next_id) {
    char filepath[256];
    strcpy(filepath, dirpath);
    strcat(filepath, "/district.cfg");
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        *severity = 1;  // default severity
        *next_id = 0;   // default next_id
        return 0;
    }
    char first;
    if (read(fd, &first, 1) == 1) {
        if (first >= '0' && first <= '9') {
            *severity = first - '0';
        } else {
            *severity = 1;
        }
    } else {
        *severity = 1;
    }

    char buf[16];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        if (sscanf(buf, "%d", next_id) != 1) {
            *next_id = 0;
        }
    } else {
        *next_id = 0;
    }

    close(fd);
    return 1;   // success
}
Report read_report() {
    Report report;
    printf("Enter coord X:");
    scanf("%f", &report.X);
    printf("Enter coord Y:");
    scanf("%f", &report.Y);
    printf("Enter category(road/lighting/flooding/other):");
    scanf("%19s", report.category);
    printf("Enter severity(1-3):");
    scanf("%d", &report.severity);
    if(report.severity < 1 ){
        report.severity = 1;
    }
    if(report.severity > 3){
        report.severity = 3;
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF);// Clear input buffer
    printf("Enter description:");
    fgets(report.description, sizeof(report.description), stdin);
    report.description[strcspn(report.description, "\n")] = '\0';// Remove trailing newline
    return report;
}
void add_report(const char *district, char *user, char *role) {
    add_directory(district);
    int severity, next_id;
    read_config(district, &severity, &next_id);
    Report report = read_report();
    report.id = next_id;
    strcpy(report.name, user);
    char filepath[256];
    strcpy(filepath, district);
    size_t len = strlen(filepath);
    if (len > 0 && filepath[len - 1] != '/') {
        strcat(filepath, "/");
    }
    strcat(filepath, "reports.dat");
    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0664);
    
    time_t currentTime;
    time(&currentTime);
    strcpy(report.timestamp, ctime(&currentTime));
    if (fd != -1) {
        write(fd, &report, sizeof(Report));
        close(fd);
        update_config(district, severity, next_id + 1);
    } else {
        perror("open failed for reports.dat");
    }
    strcpy(filepath, district);
    strcat(filepath, "/logged_district");
    fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd != -1) {
        char buf[512];
        int buflen = snprintf(buf, sizeof(buf), "User: %s Role: %s Timestamp: %s\n", user, role, ctime(&currentTime));
        write(fd, buf, buflen);
        close(fd);
    } else {
        perror("open failed for logged_district");
    }
}
void list_reports(const char *district) {
    char filepath[256];
    strcpy(filepath, district);
    size_t len = strlen(filepath);
    if (len > 0 && filepath[len - 1] != '/') {
        strcat(filepath, "/");
    }
    strcat(filepath, "reports.dat");
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("open failed for reports.dat");
        return;
    }
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        close(fd);
        return;
    }
    print_permissions(filepath);
    printf("Size of '%s': %ld bytes\n", filepath, st.st_size);
    printf("Last modified: %s", ctime(&st.st_mtime));
    Report report;
    printf("\nReports in district '%s':\n", district);
    while (read(fd, &report, sizeof(Report)) == sizeof(Report)) {
        printf("ID: %d\n", report.id);
        printf("Inspector: %s\n", report.name);
        printf("Coordinates: (%.2f, %.2f)\n", report.X, report.Y);
        printf("Category: %s\n", report.category);
        printf("Severity: %d\n", report.severity);
        printf("Description: %s\n", report.description);
        printf("Timestamp: %s\n", report.timestamp);
        printf("-------------------------\n");
    }
    close(fd);
}
void view_report(const char *district, int report_id) {
    char filepath[256];
    strcpy(filepath, district);
    size_t len = strlen(filepath);
    if (len > 0 && filepath[len - 1] != '/') {
        strcat(filepath, "/");
    }
    strcat(filepath, "reports.dat");
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("open failed for reports.dat");
        return;
    }
    int x = 0;
    Report report;
    while (read(fd, &report, sizeof(Report)) == sizeof(Report)) {
        if (report.id == report_id) {
            printf("ID: %d\n", report.id);
            printf("Inspector: %s\n", report.name);
            printf("Coordinates: (%.2f, %.2f)\n", report.X, report.Y);
            printf("Category: %s\n", report.category);
            printf("Severity: %d\n", report.severity);
            printf("Description: %s\n", report.description);
            printf("Timestamp: %s\n", report.timestamp);
            x = 1;
            break;
        }
    }
    if (!x) {
        printf("Report with ID %d not found in district '%s'.\n", report_id, district);
    }
    close(fd);
}
void remove_report(const char *district, int report_id) {
    char filepath[256];
    strcpy(filepath, district);
    size_t len = strlen(filepath);
    if (len > 0 && filepath[len - 1] != '/') {
        strcat(filepath, "/");
    }
    strcat(filepath, "reports.dat");
    int fd = open(filepath, O_RDWR);
    if (fd == -1) {
        perror("open failed for reports.dat");
        return;
    }
    int x = 0;
    Report report;
    
     while (read(fd, &report, sizeof(Report)) == sizeof(Report)) {
        if (report.id == report_id) {
            x = 1;
            off_t write_pos = lseek(fd, -(off_t)sizeof(Report), SEEK_CUR);

            Report buffer;
            while (read(fd, &buffer, sizeof(Report)) == sizeof(Report)) {
                lseek(fd, write_pos, SEEK_SET);
                write(fd, &buffer, sizeof(Report));
                write_pos += sizeof(Report);
                lseek(fd, write_pos + sizeof(Report), SEEK_SET);
            }

            ftruncate(fd, write_pos);
            break;
        }
    }
    if (!x) {
        printf("Report with ID %d not found in district '%s'.\n", report_id, district);
    }
    close(fd);
}
void change_config(const char *district, int new_severity) {
    int severity, next_id;
    if (read_config(district, &severity, &next_id)) {
        update_config(district, new_severity, next_id);
    } else {
        printf("Failed to read config for district '%s'.\n", district);
    }
}



int main(int argc, char *argv[]){
    if(argc < 6){// Minimum arguments
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
    if (strcmp(argv[5], "--add") == 0) {
        char district[20];
        strcpy(district, argv[6]);
        add_report(district, user, role);
    }
    else if (strcmp(argv[5], "--list") == 0) {
        list_reports(argv[6]);
    }
    else if (strcmp(argv[5], "--view") == 0) {
        int report_id = atoi(argv[7]);
        view_report(argv[6], report_id);
    }
    else if (strcmp(argv[5], "--remove") == 0) {
        int report_id = atoi(argv[7]);
        remove_report(argv[6], report_id);
    }
    else if (strcmp(argv[5], "--update-config") == 0) {
        int new_severity = atoi(argv[7]);
        change_config(argv[6], new_severity);
    }
    
    // ./city_manager --role manager --user alice --add downtown
    return 0;
}