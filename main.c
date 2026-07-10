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
 states modified by signal handlers.
 */
volatile int g_run = 1;
volatile int g_reload = 0;

/* configuration */
int g_level = 1;
int g_syslog = 0;
FILE *g_file = NULL;

static void handle_sig(int sig)
{
	if (sig == SIGTERM || sig == SIGINT)
		g_run = 0;
	else if (sig == SIGHUP)
		g_reload = 1;
}

static void do_log(int level, const char *msg)
{
	if (level > g_level)
		return;

	if (g_syslog) {
		int sys_lvl = LOG_INFO;

		if (level == 0)
			sys_lvl = LOG_ERR;
		else if (level == 2)
			sys_lvl = LOG_DEBUG;
		
		syslog(sys_lvl, "%s", msg);
	} else if (g_file) {
		fprintf(g_file, "%s", msg);
		fflush(g_file);
	}
}

static int daemonize(void)
{
	pid_t pid;
	int i;
	int fd;

	pid = fork();
	if (pid < 0)
		return -1;
	if (pid > 0)
		exit(EXIT_SUCCESS);

	if (setsid() < 0)
		return -1;

	pid = fork();
	if (pid < 0)
		return -1;
	if (pid > 0)
		exit(EXIT_SUCCESS);

	umask(0);

	if (chdir("/") < 0)
		return -1;

	for (i = 0; i < 1024; i++) {
		if (g_file && i == fileno(g_file))
			continue;
		close(i);
	}

	fd = open("/dev/null", O_RDWR);
	if (fd >= 0) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > 2)
			close(fd);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct sigaction sa;
	int opt;

	/* parse arguments -o and -l */
	while ((opt = getopt(argc, argv, "o:l:")) != -1) {
		if (opt == 'o') {
			if (strcmp(optarg, "syslog") == 0) {
				g_syslog = 1;
			} else {
				g_file = fopen(optarg, "a");
				if (!g_file)
					return EXIT_FAILURE;
			}
		} else if (opt == 'l') {
			g_level = atoi(optarg);
		}
	}

	/* become a daemon */
	if (daemonize() < 0)
		return EXIT_FAILURE;

	/* setup signal handling */
	sa.sa_handler = handle_sig;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);

	if (g_syslog)
		openlog("mydaemon", LOG_PID, LOG_DAEMON);

	do_log(1, "[INFO] Daemon started.\n");

	/* main background loop */
	while (g_run) {
		if (g_reload) {
			do_log(1, "[INFO] Reloading configuration - SIGHUP received.\n");
			g_reload = 0;
		}

		sleep(5);
		do_log(2, "[DEBUG] Daemon is running.\n");
	}

	/* shutdown */
	do_log(1, "[INFO] Shutdown signal received. Daemon exiting.\n");

	if (g_syslog)
		closelog();
	if (g_file)
		fclose(g_file);

	return EXIT_SUCCESS;
}