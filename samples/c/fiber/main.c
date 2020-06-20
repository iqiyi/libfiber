#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "fiber/libfiber.h"

#include "../util.h"

#define FIBER_STACK_SIZE	128000

static void fiber1(ACL_FIBER *fb, void *ctx) {
	int i = *((int*) ctx);

	(void) fb;
	free(ctx);

	printf("fiber-%d running, i=%d\r\n", acl_fiber_self(), i);
}

static void schedule_one(void) {
	int i;
	for (i = 0; i < 10; i++) {
		int *ctx = malloc(sizeof(int));
		*ctx = i;
		acl_fiber_create(fiber1, ctx, FIBER_STACK_SIZE);
	}

	acl_fiber_schedule();
}

static void schedule_two(void) {
	int i;

	acl_fiber_schedule_init(1);

	for (i = 0; i < 10; i++) {
		int *ctx = malloc(sizeof(int));
		*ctx = i;
		acl_fiber_create(fiber1, ctx, FIBER_STACK_SIZE);
	}
}

struct FIBER_CTX {
	int nloop;
	long long count;
};

static void fiber2(ACL_FIBER *fb, void *ctx) {
	struct FIBER_CTX *fc = (struct FIBER_CTX *) ctx;
	int i;
	(void) fb;

	for (i = 0; i < fc->nloop; i++) {
		if (i < 5) {
			printf("fiber-%d, i=%d\r\n", acl_fiber_self(), i);
		}
		++fc->count;
		acl_fiber_yield();
	}
}

static void schedule_bench(int nfiber, int nloop) {
	struct timeval begin, end;
	struct FIBER_CTX fc;
	int i;

	fc.nloop = nloop;
	fc.count = 0;

	acl_fiber_schedule_init(0);
	gettimeofday(&begin, NULL);

	for (i = 0; i < nfiber; i++) {
		acl_fiber_create(fiber2, &fc, FIBER_STACK_SIZE);
	}

	acl_fiber_schedule();

	gettimeofday(&end, NULL);

	show_speed(&begin, &end, fc.count);
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
