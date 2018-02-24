#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include "../patch.h"
#include "libgo/libgo.h"

static void doit(SOCKET s)
{
	char buf[8192];
	int  sz;

	while (1) {
		sz = read(s, buf, sizeof(buf) - 1);
		if (sz == 0) {
			break;
		} else if (sz < 0) {
			if (errno == EAGAIN || errno == EINTR) {
				continue;
			}
			break;
		}

		buf[sz] = 0;
		if (write(s, buf, sz) != sz) {
			break;
		}
	}

	close(s);
}

static void do_server(SOCKET ls)
{
	while (1) {
		SOCKET s = socket_accept(ls);
		printf("accept one %d\r\n", s);
		if (s != INVALID_SOCKET) {
			go [=] { doit(s); };
		} else {
			printf("accept error %s\r\n", strerror(errno));
			break;
		}
	}

	close(ls);
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

	go[=] { do_server(ls); };

	g_Scheduler.RunUntilNoTask();
	return 0;
}
