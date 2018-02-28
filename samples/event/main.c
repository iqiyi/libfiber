#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "fiber/libfiber.h"

static int __nthreads = 2;
static int __nloop    = 1000000;
static int __delay    = 0;

static long long __counter = 0;

static void *thread_main(void *ctx)
{
	ACL_FIBER_EVENT *ev = (ACL_FIBER_EVENT *) ctx;
	int i;

	for (i = 0; i < __nloop; i++) {
		if (acl_fiber_event_wait(ev) == -1)
			abort();
		if (__delay > 0)
			usleep(__delay * 1000);
		__counter++;
		if (acl_fiber_event_notify(ev) == -1)
			abort();
	}

	return NULL;
};

//////////////////////////////////////////////////////////////////////////////

static void usage(const char* procname)
{
	printf("usage: %s -h [help]\r\n"
		" -t nthreads[default: 2]\r\n"
		" -n nloop[default: 2]\r\n"
		" -d delay[default: 100 ms]\r\n",
		procname);
}

int main(int argc, char *argv[])
{
	int  ch, i;
	ACL_FIBER_EVENT *event = acl_fiber_event_create();
#define MAX_THREADS	100
	pthread_t threads[MAX_THREADS];
	pthread_attr_t attr;

	while ((ch = getopt(argc, argv, "ht:n:d:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 't':
			__nthreads = atoi(optarg);
			if (__nthreads > MAX_THREADS)
				__nthreads = MAX_THREADS;
			break;
		case 'n':
			__nloop = atoi(optarg);
			break;
		case 'd':
			__delay = atoi(optarg);
			break;
		default:
			break;
		}
	}

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, 0);

	for (i = 0; i < __nthreads; i++) {
		pthread_create(&threads[i], &attr, thread_main, event);
	}

	for (i = 0; i < __nthreads; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("all over, thread=%d, nloop=%d, counter=%lld\r\n",
		__nthreads, __nloop, __counter);

	return 0;
}
