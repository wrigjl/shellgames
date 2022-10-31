
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <termios.h>
#include <pty.h>
#include <signal.h>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <string>

#define TEMPFILE_TEMPLATE "/tmp/ttylog.XXXXXX"
#define MAX_DATA (2*1024*1024)
#define S3_ROOT "s3://saintcon-hc-2022-jlw-store/sh"

uint64_t random_token;

int
arettys(void) {
	int xtty[3];

	for (int i = 0; i < 3; i++) {
		xtty[i] = isatty(i);
		if (xtty[i] == -1)
			return -1;
		if (xtty[0] != xtty[i]) {
			errno = ENOTTY;
			return -1;
		}
	}
	return xtty[0];
}

sig_atomic_t alarm_bell = 0;
sig_atomic_t hangup_bell = 0;

void
alarm_clock(int signo) {
	alarm_bell = 1;
}

void
hangup(int signo) {
	hangup_bell = 1;
}

char *
open_log(const char *tmplate, int *fdp) {
	int fd;

	char *name = strdup(tmplate);
	if (name == NULL)
		return name;
	fd = mkstemp(name);
	if (fd == -1) {
		free(name);
		return NULL;
	}
	*fdp = fd;
	return name;
}

int
write_all(int fd, const void *v, ssize_t len) {
	const char *p = (const char *)v;
	int remaining = len;
	remaining = len;
	while (remaining) {
		len = write(fd, p, remaining);
		if (len < 0)
			return len;
		remaining -= len;
		p += len;
	}
	return 0;
}

int
send_log(const char *name, int num, time_t t) {
	char fname[256];
	pid_t pid;

	int devnull = open("/dev/null", O_RDWR, 0);
	if (devnull == -1) {
		warn("/dev/null");
		return -1;
	}
	
	snprintf(fname, sizeof(fname), "%s/%lu-%lx-%d",
		 S3_ROOT, t, random_token, num);

	pid = fork();
	if (pid == -1) {
		warn("fork");
		return -1;
	}
	if (pid == 0) {
		dup2(devnull, 0);
		dup2(devnull, 1);
		dup2(devnull, 2);
		if (devnull > 2)
			close(devnull);
		execl("/bin/aws", "aws", "s3", "cp", name, fname, NULL);
		warn("execl");
		_exit(1);
	}

	waitpid(pid, NULL, 0);
	return 0;
}

int
main() {
	int ttys, masterfd, slavefd;
	struct termios termattr;
	struct winsize winsz;
	struct sigaction sa;
	pid_t pid;
	char *log1name = NULL, *log2name = NULL;
	int log1fd = -1, log2fd = -1;
	ssize_t rxdata = 0, txdata = 0;

	ttys = arettys();
	if (ttys == -1)
		err(1, "ttys");
	if (ttys == 0)
		errx(1, "sorry, ttys are required");

	if (tcgetattr(0, &termattr) == -1)
		err(1, "tcgetattr");

	if (ioctl(0, TIOCGWINSZ, &winsz) == -1)
		err(1, "tiocgwinsz");

	if (openpty(&masterfd, &slavefd, NULL, &termattr, &winsz) == -1)
		err(1, "openpty");

	cfmakeraw(&termattr);
	if (tcsetattr(0, TCSANOW, &termattr) == -1)
		err(1, "tcsetattr");

	pid = fork();
	if (pid == -1)
		err(1, "fork");
	if (pid == 0) {
		close(masterfd);

		if (dup2(slavefd, 0) == -1)
			err(1, "dup2");
		if (dup2(slavefd, 1) == -1)
			err(1, "dup2");
		if (dup2(slavefd, 2) == -1)
			err(1, "dup2");

		if (slavefd > 2)
			close(slavefd);
		execl("/bin/sudo", "sudo", "/bin/shell2docker", NULL);
		err(1, "sudo");
	}

	// We're in the parent

	close(slavefd);

	slavefd = open("/dev/urandom", O_RDONLY, 0);
	if (slavefd == -1)
		err(1, "/dev/urandom");
	if (read(slavefd, &random_token, sizeof(random_token)) !=
	    sizeof(random_token))
		err(1, "read token");
	close(slavefd);

	sa.sa_handler = SIG_DFL;
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		err(1, "sigchild");

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = alarm_clock;
	sa.sa_flags = 0;
	if (sigaction(SIGALRM, &sa, NULL) == -1)
		err(1, "sigaction");
	alarm(60);

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = alarm_clock;
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) == -1)
		err(1, "sigaction");

	log1name = open_log(TEMPFILE_TEMPLATE, &log1fd);
	if (log1name == NULL)
		err(1, "error opening log");

	log2name = open_log(TEMPFILE_TEMPLATE, &log2fd);
	if (log2name == NULL)
		err(1, "error opening log");

	while (1) {
		char buf[128];
		ssize_t len;
		int r;
		fd_set fds;
		struct timeval timeout;

		if (alarm_bell != 0 ||
		    txdata > MAX_DATA ||
		    rxdata > MAX_DATA)
			break;

		FD_ZERO(&fds);
		FD_SET(0, &fds);	// stdin
		FD_SET(masterfd, &fds);	// pty
		auto maxfd = std::max(masterfd, 0) + 1;
		timeout.tv_usec = 0;
		timeout.tv_sec = 5;

		r = select(maxfd, &fds, NULL, NULL, &timeout);
		if (r == -1) {
			if (errno == EINTR)
				continue;
		}
		if (r == 0)
			continue;

		if (FD_ISSET(masterfd, &fds)) {
			// data from the tty, xfer to stdout and log
			len = read(masterfd, buf, sizeof(buf));
			if (len == -1) {
				if (errno == EINTR)
					break;
				break;
			}
			if (len == 0)
				break;

			if (write_all(log1fd, buf, len))
				break;
			if (write_all(1, buf, len))
				break;

			txdata += len;
		}

		if (FD_ISSET(0, &fds)) {
			// data from stdin, xfer to tty and log
			len = read(0, buf, sizeof(buf));
			if (len == -1) {
				if (errno == EINTR)
					break;
				break;
			}
			if (len == 0)
				break;

			if (write_all(log2fd, buf, len))
				break;
			if (write_all(masterfd, buf, len))
				break;

			rxdata += len;
		}
	}

	if (hangup_bell != 0)
		kill(pid, SIGHUP);

	close(masterfd);

	time_t t = time(NULL);
	send_log(log1name, 1, t);
	unlink(log1name);

	send_log(log2name, 2, t);
	unlink(log2name);

	return 0;
}
