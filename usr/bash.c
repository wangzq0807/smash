#include "unistd.h"
#include "stdio.h"
#include "string.h"

#define HOSTNAME    ("smash$ ")

void change_dir(char *cmd);
void run_cmd(char *cmd);
char *split_cmd(char *cmd);

int
main(int argc, const char **argv)
{
    int mypid = getpid();
    if (mypid != 1) {
        exit(0);
    }
    char buf[1025] = {0};
    int fd = 0;
    fd = open("/dev/tty", O_RDWR, 0);
    if (fd == -1)
        return 0;

    while (1) {
        printf(HOSTNAME);
        int cnt = read(fd, buf, 1024);
        if (cnt == -1 || cnt == 0)
            continue;
        if (buf[cnt-1] == '\r') {
            buf[cnt] = 0;
        }

        int pipefd[2] = {-1, -1};
        int firstcmd = 1;
        int prepipefd[2] = {-1, -1};
        char *cmds = strim(buf, " \r\n\t");
        while (*cmds) {
            char *cmd = strsep(&cmds, "|");
            cmd = strim(cmd, " \r\n\t");

            if (strncmp(buf, "cd ", 3) == 0) {
                change_dir(buf+3);
                continue;
            }

            if (*cmds != 0) {
                pipe(pipefd);
            }

            int pid = fork();
            if (pid == 0) {
                if (*cmds != 0) {
                    close(pipefd[0]);
                    close(stdout);
                    int dufd = dup(pipefd[1]);
                    if (dufd != stdout)
                        printf("redirect stdout failed\n");
                }
                if (firstcmd != 1) {
                    close(prepipefd[1]);
                    close(stdin);
                    int dufd = dup(prepipefd[0]);
                    if (dufd != stdin)
                        printf("redirect stdin failed\n");
                }
                run_cmd(cmd);
            }
            else if (pid > 0) {
                waitpid(pid, NULL, 0);
                if (firstcmd != 1) {
                    close(prepipefd[0]);
                    close(prepipefd[1]);
                }
                firstcmd = 0;
                prepipefd[0] = pipefd[0];
                prepipefd[1] = pipefd[1];
            }
        }
    }
    close(fd);

    return 0;
}

void
run_cmd(char *buf)
{
    char execfile[128] = "/bin/";
    char *params[11] = { NULL };
    char *tmp = buf;
    for (int i = 0; i < 10; ++i) {
        char *token = strsep(&tmp, " \t");
        if (token == NULL)  break;
        params[i] = token;
    }
    if (params[0][0] == 0)
        exit(0);

    if (params[0][0] == '/' || params[0][0] == '.') {
        strcpy(execfile, params[0]);
    }
    else {
        strcat(execfile, params[0]);
    }

    if (execve(execfile, params+1, NULL) == -1) {
        printf("exec %s failed\n", execfile);
        exit(0);
    }
    exit(0);
}

void
change_dir(char *dir)
{
    dir = strim(dir, " \r\n\t");
    int r = chdir(dir);
    if (r == -1) {
        printf("failed to open target dir\n");
    }
}
