#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <getopt.h>
#include "fiber/libfiber.hpp"
#include "fiber/go_fiber.hpp"

#include "../util.h"

static void fiber1(int i) {
	printf("fiber-%d running, i=%d\r\n", acl::fiber::self(), i);
}

static void schedule_one(void) {
	std::vector<acl::fiber*> fibers;
	for (int i = 0; i < 10; i++) {
		go[&] { fiber1(i); };
	}

	acl::fiber::schedule_with(acl::FIBER_EVENT_T_KERNEL);
}

static void schedule_two(void) {
	acl::fiber::init(acl::FIBER_EVENT_T_KERNEL, true);

	for (int i = 0; i < 10; i++) {
		go[&] { fiber1(i); };
	}
}

static void fiber2(int n, long long& count) {
	for (int i = 0; i < n; i++) {
		if (i < 5) {
			printf("fiber-%d, i=%d\r\n", acl::fiber::self(), i);
		}
		++count;
		acl::fiber::yield();
	}
}

static void schedule_bench(int nfiber, int nloop) {
	acl::fiber::init(acl::FIBER_EVENT_T_KERNEL, false);

	struct timeval begin;
	gettimeofday(&begin, NULL);

	long long count = 0;
	for (int i = 0; i < nfiber; i++) {
		go[&] { fiber2(nloop, count); };
	}

	acl::fiber::schedule();

	struct timeval end;
	gettimeofday(&end, NULL);

	show_speed(begin, end, count);
}

static void usage(const char* procname) {
	printf("usage: %s -h [help] -c fibers -n loop\r\n", procname);
}

int main(int argc, char* argv[]) {
	int ch, nfiber = 10, nloop = 1000000;

	while ((ch = getopt(argc, argv, "hc:n:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
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

	printf("schedule one\r\n");
	schedule_one();

	printf("\r\n");
	printf("schedule two\r\n");
	schedule_two();

	printf("\r\n");
	printf("schedule benchmark\r\n");
	schedule_bench(nfiber, nloop);
	return 0;
}
