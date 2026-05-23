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

#define MAX_DISTRICTS 64
#define MAX_LINE 256

int directory_exists(const char *name) {
    struct stat st;
    return (stat(name, &st) == 0 && S_ISDIR(st.st_mode));
}

void hub_mon(){
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe failed");
        return;
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        close(fd[0]);
        close(fd[1]);
        return;
    } else if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);
        execl("./monitor_reports", "./monitor_reports", NULL);
        perror("execl failed");
        exit(1);
    } else {
        close(fd[1]);
        char buffer[256];
        int n;
        while ((n = read(fd[0], buffer, sizeof(buffer))) > 0) {
            buffer[n] = '\0';
            printf("Monitor: %s", buffer);
        }   
        close(fd[0]);
        wait(NULL);
    }
}

void calc_scores( char *district) {
    
        if(!directory_exists(district)) {
            fprintf(stderr, "District '%s' does not exist, skipping...\n", district);
            return;
        }
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe failed");
            return;
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            close(fd[0]);
            close(fd[1]);
            return;
        } else if (pid == 0) {
            close(fd[0]);
            if(dup2(fd[1], 1) == -1) {
                perror("dup2 failed");
                exit(1);
            }
            close(fd[1]);
            execl("./scorer", "./scorer", district, NULL);
            perror("execl failed");
            exit(1);
        }
        else {
            close(fd[1]);
            char buffer[256];
            int n;
            printf("Scores for district '%s':\n", district);
            while ((n = read(fd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[n] = '\0';
                printf("%s", buffer);
            }
            close(fd[0]);
        }
    
}


static char *read_line(char *buf, int size) {
    if (!fgets(buf, size, stdin))
        return NULL;
    buf[strcspn(buf, "\n")] = '\0';   /* strip newline */
    return buf;
}

int main(void) {
    char line[MAX_LINE];

    printf("Enter command: ");
    fflush(stdout);

    if (!read_line(line, sizeof(line))) {
        fprintf(stderr, "Failed to read command.\n");
        return 1;
    }

    if (strcmp(line, "start_monitor") == 0) {
        printf("Starting City Hub...\n");
        hub_mon();

    } else if (strcmp(line, "calc_scores") == 0) {
        
        printf("Enter districts (one per line, empty line to finish):\n");
        fflush(stdout);

        

    while (1) {
        printf("  district: ");
        fflush(stdout);

        if (!read_line(line, sizeof(line)) || line[0] == '\0')
            break;

        if(strcmp(line,"end") == 0)
            break;
        if (!directory_exists(line)) {
            fprintf(stderr, "Warning: '%s' is not a valid district folder.\n", line);
            continue;
        }

        calc_scores(line);
    }

    } else {
        fprintf(stderr, "Unknown command: '%s'\n", line);
        return 1;
    }

    return 0;
}