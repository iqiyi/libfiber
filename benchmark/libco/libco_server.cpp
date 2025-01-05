#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stack>

#include "../patch.h"
#include "libco/co_routine.h"

using namespace std;
struct task_t
{
	stCoRoutine_t *co;
	int fd;
};

static stack<task_t*> g_readwrite;
static int __listen_fd = -1;

static int SetNonBlock(int iSock)
{
	int iFlags;

	iFlags = fcntl(iSock, F_GETFL, 0);
	iFlags |= O_NONBLOCK;
	iFlags |= O_NDELAY;
	return fcntl(iSock, F_SETFL, iFlags);
}

static void *readwrite_routine(void *arg)
{
	task_t *co = (task_t*)arg;
	char buf[8192];
	int  sz;

	co_enable_hook_sys();

	while (1) {
		if (co->fd == -1) {
			g_readwrite.push(co);
			co_yield_ct();
			continue;
		}

		int fd = co->fd;
		co->fd = -1;

		for (;;) {
			struct pollfd pf;
			pf.fd = fd;
			pf.events = (POLLIN|POLLERR|POLLHUP);
			co_poll(co_get_epoll_ct(), &pf, 1, 1000);

			sz = read(fd, buf, sizeof(buf) - 1);
			if (sz == 0) {
				close(fd);
				break;
			} else if (sz < 0) {
				if (errno == EAGAIN || errno == EINTR) {
					continue;
				}
				close(fd);
				break;
			}

			buf[sz] = 0;
			if (write(fd, buf, sz) != sz) {
				close(fd);
				break;
			}
		}
	}
}

extern int co_accept(int fd, struct sockaddr *addr, socklen_t *len);

static void *accept_routine(void *)
{
	co_enable_hook_sys();

	while (1) {
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		socklen_t len = sizeof(addr);
		int fd = co_accept(__listen_fd, (struct sockaddr *)&addr, &len);
		if (fd < 0) {
			struct pollfd pf;
			pf.fd = __listen_fd;
			pf.events = (POLLIN|POLLERR|POLLHUP);
			co_poll( co_get_epoll_ct(), &pf, 1, 1000);
			continue;
		}

		if(g_readwrite.empty())  {
			close(fd);
			continue;
		}

		SetNonBlock(fd);
		task_t *co = g_readwrite.top();
		co->fd = fd;
		g_readwrite.pop();
		co_resume(co->co);
	}
}

static void usage(const char *procname)
{
	printf("usage: %s -h[help] -s ip -p port\r\n", procname);
}

int main(int argc, char *argv[])
{
	char ip[128];
	int port = 9001, ch;

	snprintf(ip, sizeof(ip), "127.0.0.1");

	while ((ch = getopt(argc, argv, "hs:p:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			snprintf(ip, sizeof(ip), "%s", optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		default:
			break;
		}
	}

	SOCKET ls = socket_listen(ip, port);
	if (ls == INVALID_SOCKET) {
		printf("tcplisten error %s\r\n", strerror(errno));
		return 1;
	}
	printf("listen %s:%d ok\r\n", ip, port);
	__listen_fd = ls;
	SetNonBlock(__listen_fd);

	for (int i = 0; i < 1024; i++) {
		task_t *task = (task_t *) calloc(1, sizeof(task_t));
		task->fd = -1;
		co_create(&(task->co), NULL, readwrite_routine, task);
		co_resume(task->co);
	}

	stCoRoutine_t *accept_co = NULL;
	co_create(&accept_co, NULL, accept_routine, 0);
	co_resume(accept_co);
	co_eventloop(co_get_epoll_ct(), 0, 0);

	return 0;
}
