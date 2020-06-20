#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <getopt.h>
#include "fiber/libfiber.hpp"

#include "../util.h"

class myfiber : public acl::fiber {
public:
	myfiber(int i) : i_(i) {}

	~myfiber(void) {}

protected:
	// @override
	void run(void) {
		printf("fiber-%d running, i=%d\r\n", acl::fiber::self(), i_);
	}

private:
	int i_;
};

static void free_fibers(std::vector<acl::fiber*>& fibers) {
	for (std::vector<acl::fiber*>::iterator it = fibers.begin();
		it != fibers.end(); ++it) {
		delete *it;
	}
	fibers.clear();
}

static void schedule_one(void) {
	std::vector<acl::fiber*> fibers;
	for (int i = 0; i < 10; i++) {
		acl::fiber* fb = new myfiber(i);
		fibers.push_back(fb);
		fb->start();
	}

	acl::fiber::schedule_with(acl::FIBER_EVENT_T_KERNEL);
	free_fibers(fibers);
}

static void schedule_two(void) {
	acl::fiber::init(acl::FIBER_EVENT_T_KERNEL, true);

	std::vector<acl::fiber*> fibers;
	for (int i = 0; i < 10; i++) {
		acl::fiber* fb = new myfiber(i);
		fibers.push_back(fb);
		fb->start();
	}
	free_fibers(fibers);
}

class myfiber1 : public acl::fiber {
public:
	myfiber1(int n) : n_(n) {}
	~myfiber1(void) {}

private:
	int n_;

	void run(void) {
		for (int i = 0; i < n_; i++) {
			if (i < 5) {
				printf("fiber-%d, i=%d\r\n",
					acl::fiber::self(), i);
			}
			acl::fiber::yield();
		}
	}
};

static void schedule_bench(int nfiber, int nloop) {
	acl::fiber::init(acl::FIBER_EVENT_T_KERNEL, false);
	std::vector<acl::fiber*> fibers;

	struct timeval begin;
	gettimeofday(&begin, NULL);

	for (int i = 0; i < nfiber; i++) {
		acl::fiber* fb = new myfiber1(nloop);
		fibers.push_back(fb);
		fb->start();
	}

	acl::fiber::schedule();

	struct timeval end;
	gettimeofday(&end, NULL);

	long long count = ((long long) nfiber) *((long long) nloop);
	show_speed(begin, end, count);
	free_fibers(fibers);
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
	schedule_bench(nfiber, nloop);
	return 0;
}
