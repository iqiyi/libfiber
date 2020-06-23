#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "fiber/go_fiber.hpp"

#include "../patch.h"
#include "../util.h"

static void fiber_client(const std::string& ip, int port, int n, long long& count) {
	SOCKET fd = socket_connect(ip.c_str(), port);
	if (fd == INVALID_SOCKET) {
		printf("connect %s %d error %s\r\n", ip.c_str(), port,
			acl::fiber::last_serror());
		return;
	}

	printf("connect %s %d ok, fd=%d\r\n", ip.c_str(), port, fd);

	const char* s = "hello world!\r\n";
	char buf[8192];
	for (int i = 0; i < n; i++) {
		if (write(fd, s, strlen(s)) < 0) {
			printf("send error %s\r\n", acl::fiber::last_serror());
			break;
		}

		int ret = read(fd, buf, sizeof(buf) - 1);
		if (ret == 0) {
			break;
		}
		if (ret == -1) {
			if (errno == EINTR) {
				continue;
			}
			printf("read error %s\r\n", acl::fiber::last_serror());
			break;
		}
		count++;
	}

	printf("close fd=%d\r\n", fd);
	socket_close(fd);
}

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s server_ip -p server_port\r\n", procname);
}

int main(int argc, char* argv[]) {
	std::string ip = "127.0.0.1";
	int port = 8192, ch, nfiber = 100, nloop = 10000;

	while ((ch = getopt(argc, argv, "hs:p:c:n:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			ip = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'c':
			nfiber = atoi(optarg);
			break;
		case 'n':
			nloop = atoi(optarg);
			break;
		default:
			break;
		}
	}

	socket_init();

	struct timeval begin;
	gettimeofday(&begin, NULL);
	long long count = 0;

	for (int i = 0; i < nfiber; i++) {
		go[&] {
			fiber_client(ip, port, nloop, count);
		};
	}

	acl::fiber::schedule_with(acl::FIBER_EVENT_T_KERNEL);

	struct timeval end;
	gettimeofday(&end, NULL);

	show_speed(begin, end, count);
	socket_end();
	return 0;
}
