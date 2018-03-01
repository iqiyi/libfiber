#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "fiber/libfiber.h"

static size_t __stack_size = 64000;
static int __nthreads = 2;
static int __nfibers  = 1000;
static int __nloop    = 10000;

static void fiber_main(ACL_FIBER *fb, void *ctx)
{
	int i;

	(void) fb;
	(void) ctx;

	for (i = 0; i < __nloop; i++) {
		acl_fiber_yield();
	}
}

static void *thread_main(void *ctx)
{
	int i;

	for (i = 0; i < __nfibers; i++) {
		acl_fiber_create(fiber_main, ctx, __stack_size);
	}

	acl_fiber_schedule();

	printf("thread-%lu: over now, nfibers=%d, nloop=%lld\r\n",
		(unsigned long) pthread_self(), __nfibers,
		((long long) __nfibers) * ((long long) __nloop));

	return NULL;
};

//////////////////////////////////////////////////////////////////////////////

static void usage(const char* procname)
{
	printf("usage: %s -h [help]\r\n"
		" -t nthreads [default: 2]\r\n"
		" -c nfibers [default: 1000]\r\n"
		" -z stack_size [default: 64000]\r\n"
		" -n nloop [default: 1000]\r\n",
		procname);
}

int main(int argc, char *argv[])
{
	int  ch, i;
#define MAX_THREADS	100
	pthread_t threads[MAX_THREADS];

	while ((ch = getopt(argc, argv, "ht:c:z:n:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 't':
			__nthreads = atoi(optarg);
			if (__nthreads > MAX_THREADS)
				__nthreads = MAX_THREADS;
			break;
		case 'c':
			__nfibers = atoi(optarg);
			break;
		case 'z':
			__stack_size = atol(optarg);
			break;
		case 'n':
			__nloop = atoi(optarg);
			break;
		default:
			break;
		}
	}

	for (i = 0; i < __nthreads; i++) {
		pthread_create(&threads[i], NULL, thread_main, NULL);
	}

	for (i = 0; i < __nthreads; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("all over, thread=%d, nloop=%d\r\n", __nthreads, __nloop);

	return 0;
}
