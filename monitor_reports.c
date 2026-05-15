#define _POSIX_C_SOURCE 200809L 
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int check_monitor_on(){
    int fd = open(".monitor_pid", O_RDONLY);
    if (fd == -1) {
        return 0; // Monitor not running
    }
    return 1; // Monitor is running
}

void get_pid() {
    int f = open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (f == -1) {
        perror("Failed to open .monitor_pid");
        exit(1);
    }
    char buf[32];
    pid_t pid = getpid();
    int len = snprintf(buf, sizeof(buf), "%d\n", pid);
    write(f, buf, len);
    close(f);
}


static void sig_handler(int signum) {
    if (signum == SIGINT) {
        unlink(".monitor_pid");
        write(STDOUT_FILENO, "Exiting gracefully...\n", 22);
        _exit(0);
    } else if (signum == SIGUSR1) {
        write(STDOUT_FILENO, "New report added.\n", 18);
    }
}

int main(int argc, char *argv[]) {
    if(check_monitor_on()) {
        printf("Monitor is already running.\n");
        return 0;
    }
    else{
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

    get_pid();

    while (1) {
        pause();    // wait for signals
    }
    return 0;
    }
}
