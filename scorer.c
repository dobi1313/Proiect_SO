#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>

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

typedef struct {
    char name[20];
    int  score;
    int  count;
} Inspector;

#define MAX_INSPECTORS 128

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <district>\n", argv[0]);
        return 1;
    }

    const char *district = argv[1];

    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("scorer: open failed");
        return 1;
    }

    Inspector inspectors[MAX_INSPECTORS];
    int n_inspectors = 0;

    Report r;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int found = -1;
        for (int i = 0; i < n_inspectors; i++) {
            if (strncmp(inspectors[i].name, r.name, 20) == 0) {
                found = i;
                break;
            }
        }
        if (found == -1) {
            if (n_inspectors >= MAX_INSPECTORS) continue;
            strncpy(inspectors[n_inspectors].name, r.name, 19);
            inspectors[n_inspectors].name[19] = '\0';
            inspectors[n_inspectors].score = 0;
            inspectors[n_inspectors].count = 0;
            found = n_inspectors++;
        }
        inspectors[found].score += r.severity;
        inspectors[found].count++;
    }
    close(fd);

    printf("District: %s\n", district);
    if (n_inspectors == 0) {
        printf("  (no reports)\n");
    } else {
        for (int i = 0; i < n_inspectors; i++) {
            printf("  Inspector: %-19s  reports: %3d  workload score: %d\n",
                   inspectors[i].name,
                   inspectors[i].count,
                   inspectors[i].score);
        }
    }

    return 0;
}