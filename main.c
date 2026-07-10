#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <getopt.h>

/*
  global variables for state.
 */
volatile int g_run = 1;
volatile int g_reload = 0;

/* log variables */
int g_level = 1;
int g_syslog = 0;
FILE *g_file = NULL;

/*
  simple signal handler.
  just sets the flags --so main loop can do the work.
 */
static void handle_sig(int sig)
{
        if (sig == SIGTERM || sig == SIGINT)
                g_run = 0;
        else if (sig == SIGHUP)
                g_reload = 1;
}

int main(int argc, char *argv[])
{
        int opt;
        int i;
        int fd;
        pid_t pid;
        struct sigaction sa;

        /* Parse command line arguments */
        while ((opt = getopt(argc, argv, "o:l:")) != -1) {
                if (opt == 'o') {
                        if (strcmp(optarg, "syslog") == 0) {
                                g_syslog = 1;
                        } else {
                                g_file = fopen(optarg, "a");
                                if (!g_file)
                                        goto err_out;
                        }
                } else if (opt == 'l') {
                        g_level = atoi(optarg);
                }
        }

        /*
          daemonize process.
          doing it inline here 
         */
        pid = fork();
        if (pid < 0)
                goto err_out;
        if (pid > 0)
                exit(0);

        if (setsid() < 0)
                goto err_out;

        pid = fork();
        if (pid < 0)
                goto err_out;
        if (pid > 0)
                exit(0);

        umask(0);

        if (chdir("/") < 0)
                goto err_out;

        /* close all open files except log file */
        for (i = 0; i < 1024; i++) {
                if (g_file && i == fileno(g_file))
                        continue;
                close(i);
        }

        /* redirect stdin - stdout - stderr to ===>  /dev/null */
        fd = open("/dev/null", O_RDWR);
        if (fd >= 0) {
                dup2(fd, 0);
                dup2(fd, 1);
                dup2(fd, 2);
                if (fd > 2)
                        close(fd);
        }

        /* setup signals */
        sa.sa_handler = handle_sig;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        
        sigaction(SIGTERM, &sa, NULL);
        sigaction(SIGINT,  &sa, NULL);
        sigaction(SIGHUP,  &sa, NULL);

        if (g_syslog)
                openlog("my_daemon", LOG_PID, LOG_DAEMON);

        /*
         * main background loop
         */
        while (g_run) {
                if (g_reload) {
                        if (g_syslog && g_level >= 1) {
                                syslog(LOG_INFO, "Reloading config...\n");
                        } else if (g_file && g_level >= 1) {
                                fprintf(g_file, "Reloading config...\n");
                                fflush(g_file);
                        }
                        g_reload = 0;
                }

                sleep(5);

                if (g_syslog && g_level >= 2) {
                        syslog(LOG_DEBUG, "Daemon is running...\n");
                } else if (g_file && g_level >= 2) {
                        fprintf(g_file, "Daemon is running...\n");
                        fflush(g_file);
                }
        }

        /* cleanup */
        if (g_syslog && g_level >= 1)
                syslog(LOG_INFO, "Daemon exiting...\n");
        else if (g_file && g_level >= 1) {
                fprintf(g_file, "Daemon exiting...\n");
                fflush(g_file);
        }

        if (g_syslog)
                closelog();
        if (g_file)
                fclose(g_file);

        return 0;

/* error exit */
err_out:
        return 1;
}