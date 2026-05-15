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

void calc_scores(int district_count, char **districts) {
    for (int i = 0; i < district_count; i++) {
        if(!directory_exists(districts[i])) {
            fprintf(stderr, "District '%s' does not exist, skipping...\n", districts[i]);
            continue;
        }
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe failed");
            continue;
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            close(fd[0]);
            close(fd[1]);
            continue;
        } else if (pid == 0) {
            close(fd[0]);
            if(dup2(fd[1], 1) == -1) {
                perror("dup2 failed");
                exit(1);
            }
            close(fd[1]);
            execl("./scorer", "./scorer", districts[i], NULL);
            perror("execl failed");
            exit(1);
        }
        else {
            close(fd[1]);
            char buffer[256];
            int n;
            printf("Scores for district '%s':\n", districts[i]);
            while ((n = read(fd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[n] = '\0';
                printf("%s", buffer);
            }
            close(fd[0]);
        }
    }
    for (int i = 0; i < district_count; i++) {
        wait(NULL);
    }
}


int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <district1> [<district2> ...]\n", argv[0]);
        return 1;
    }
    if(strcmp(argv[1], "start_monitor") == 0) {
    printf("Starting City Hub...\n");
    hub_mon();
    }
    if(strcmp(argv[1], "calculate_scores") == 0) {
        if(argc < 3) {
            fprintf(stderr, "Usage: %s calculate_scores <district1> [<district2> ...]\n", argv[0]);
            return 1;
        }
        calc_scores(argc - 2, &argv[2]);
    }
    
    return 0;
}