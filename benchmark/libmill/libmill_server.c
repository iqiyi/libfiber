#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "libmill.h"

static coroutine void doit(tcpsock as)
{
	int64_t deadline = now() + 10000;
	char buf[8192];
	size_t sz;

	deadline = -1;
	while (1) {
		sz =  tcprecvuntil(as, buf, sizeof(buf) - 1, "\n", 1, deadline);
		if (sz == 0) {
			break;
		} else if (errno != 0) {
			break;
		}

		buf[sz] = 0;
		if (tcpsend(as, buf, sz, deadline) == 0) {
			break;
		}
		if (errno != 0) {
			break;
		}
		tcpflush(as, deadline);
		if (errno != 0) {
			break;
		}
	}

	tcpclose(as);
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

	ipaddr addr = iplocal(ip, port, 0);
	tcpsock ls = tcplisten(addr, 10);
	if (!ls) {
		printf("tcplisten error %s\r\n", strerror(errno));
		return 1;
	}

	printf("listen %s:%d ok\r\n", ip, port);

	while (1) {
		tcpsock as = tcpaccept(ls, -1);
		if (as) {
			go(doit(as));
		} else {
			printf("tcpaccept error %s\r\n", strerror(errno));
			break;
		}
	}

	tcpclose(ls);
	return 0;
}
