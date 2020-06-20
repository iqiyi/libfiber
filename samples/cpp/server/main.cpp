#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include "fiber/libfiber.hpp"

#include "../patch.h"

class fiber_client : public acl::fiber {
public:
	fiber_client(SOCKET fd) : fd_(fd) {}

private:
	SOCKET fd_;

	~fiber_client(void) {}

protected:
	// @override
	void run(void) {
		char buf[8192];

		while (true) {
			int ret = read(fd_, buf, sizeof(buf) - 1);
			if (ret == 0) {
				break;
			}
			if (ret == -1) {
				if (errno == EINTR) {
					continue;
				}
				printf("read error %s\r\n",
					acl::fiber::last_serror());
				break;
			}
			if (write(fd_, buf, ret) < 0) {
				printf("send error %s\r\n",
					acl::fiber::last_serror());
				break;
			}
		}

		socket_close(fd_);
		delete this;
	}
};

class fiber_listen : public acl::fiber {
public:
	fiber_listen(SOCKET lfd) : lfd_(lfd) {}

private:
	SOCKET lfd_;

	~fiber_listen(void) {}

protected:
	// @override
	void run(void) {
		while (true) {
			SOCKET cfd = socket_accept(lfd_);
			if (cfd == -1) {
				printf("accept error %s\r\n",
					acl::fiber::last_serror());
				break;
			}
			printf("accept one client fd=%d\r\n", cfd);

			acl::fiber* fb = new fiber_client(cfd);
			fb->start();
		}

		socket_close(lfd_);
		delete this;
	}
};

static void usage(const char* procname) {
	printf("usage: %s -h [help] -s ip -p port\r\n", procname);
}

int main(int argc, char* argv[]) {
	std::string ip = "127.0.0.1";
	int port = 8192, ch;

	while ((ch = getopt(argc, argv, "hs:p:")) > 0) {
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
		default:
			break;
		}
	}

	socket_init();

	SOCKET fd = socket_listen(ip.c_str(), port);
	if (fd == -1) {
		printf("listen %s %d error %s\r\n",
			ip.c_str(), port, acl::fiber::last_serror());
		return 1;
	}

	printf("listen %s %d ok\r\n", ip.c_str(), port);

	acl::fiber* fb = new fiber_listen(fd);
	fb->start();

	acl::fiber::schedule_with(acl::FIBER_EVENT_T_KERNEL);
	socket_end();
	return 0;
}
