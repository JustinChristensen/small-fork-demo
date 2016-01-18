#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>

struct args {
    int should_sleep;
    int forked_children;
};

bool parse_args(struct args *args, int argc, char **argv)
{
    bool result = true;

    const struct option options[] = {
        { "sleep", no_argument, &args->should_sleep, 1 },
        { "fork", required_argument, NULL, 'f' },
        { NULL, 0, NULL, 0 }
    };

    int fl;

    while (result && (fl = getopt_long(argc, argv, "f:", options, NULL)) != -1) {
        switch (fl) {
            case 'f':
                if ((args->forked_children = strtol(optarg, NULL, 10)) == 0) {
                    perror("argument error");
                    result = false;
                }
                break;
            case 0:
                break;
            default:
                result = false;
                break;
        }                                             
    }

    return result;
}

static int waitall(int exit_status) 
{
    bool has_children = true;

    while (has_children) {
        int wait_status;
        pid_t res = wait(&wait_status);

        if (res == -1) {
            if (errno == ECHILD) {
                has_children = false;
                exit_status = EXIT_SUCCESS;
            }
            else {
                perror("wait error");
            }
        }
        else {
            if (WIFEXITED(wait_status)) {
                printf("child process exited with %d\n", WEXITSTATUS(wait_status));
            }
            else if (WIFSIGNALED(wait_status)) {
                printf("child process signaled %d\n", WTERMSIG(wait_status));

                if (WCOREDUMP(wait_status)) {
                    printf("%s\n", "child processed created a core dump");
                }
            }
        }
    }

    return exit_status;
}

int main(int argc, char *argv[])
{
    struct args args = { 0, 1 };

    bool parse_result = parse_args(&args, argc, argv);

    if (!parse_result) {
        return EXIT_FAILURE;
    }

    argc -= optind;
    argv += optind;

    if (args.should_sleep) {
        printf("%s\n", "forked child processes will sleep...");
    }

    if (args.forked_children) {
        if (args.forked_children > 4) {
            fprintf(stderr, "%s\n", "number of forked children must be less than 5");
            return EXIT_FAILURE;
        }

        printf("%d children will be forked...\n", args.forked_children);
    }

    printf("parent process has pid: %d\n", getpid());

    while (args.forked_children-- > 0) {
        int pid = fork();        

        if (pid == 0) {
            pid_t cid = getpid();
            printf("child process created with pid: %d\n", cid);

            if (args.should_sleep) {
                printf("%s\n", "child process sleeping...");

                sigset_t sigmask;
                sigemptyset(&sigmask);

                while (1) sigsuspend(&sigmask);

                _exit(EXIT_SUCCESS);
            }
            else {
                printf("%s\n", "child process exiting normally...");
                _exit(EXIT_SUCCESS);
            }
        }
        else if (pid == -1)  {
            perror("error creating child process");
        }
    }

    return waitall(EXIT_FAILURE);
}
