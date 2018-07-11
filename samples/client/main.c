#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "fiber/libfiber.h"
#include "../patch.h"
#include "../stamp.h"

static char __server_ip[64];
static int  __server_port = 9001;

static long long int __total_count = 0;
static int __total_clients         = 0;
static int __total_error_clients   = 0;

static int __conn_timeout = 0;
static int __rw_timeout   = 0;
static int __max_loop     = 10000;
static int __max_fibers   = 100;
static int __left_fibers  = 100;
static int __read_data    = 1;
static int __stack_size   = 128000;

#if defined(_WIN32) || defined(_WIN64)
static DWORD __begin;
#else
static struct timeval __begin;
#endif

static void echo_client(SOCKET fd)
{
	char  buf[8192];
	int   ret, i;
	const char *str = "hello world\r\n";

	for (i = 0; i < __max_loop; i++) {
		if (acl_fiber_send(fd, str, strlen(str), 0) <= 0) {
			printf("write error: %s\r\n", strerror(errno));
			break;
		}

		if (!__read_data) {
			__total_count++;
			if (i % 10000 == 0)
				printf("fiber-%d: total %lld, curr %d\r\n",
					acl_fiber_self(), __total_count, i);
			if (__total_count % 10000 == 0)
				acl_fiber_yield();
			continue;
		}

		ret = acl_fiber_recv(fd, buf, sizeof(buf), 0);
		if (ret <= 0) {
			printf("read error: %s\r\n", strerror(errno));
			break;
		}

		__total_count++;
	}

	acl_fiber_close(fd);
}

static void fiber_connect(ACL_FIBER *fiber, void *ctx)
{
	SOCKET  fd = socket_connect(__server_ip, __server_port);

	(void) fiber;
	(void) ctx;

	if (fd == INVALID_SOCKET) {
		printf("fiber-%d: connect %s:%d error %s\r\n",
			acl_fiber_self(), __server_ip, __server_port,
			strerror(errno));
		__total_error_clients++;
	} else {
		__total_clients++;
		printf("fiber-%d: connect %s:%d ok, clients: %d, fd: %d\r\n",
			acl_fiber_self(), __server_ip, __server_port,
			__total_clients, fd);
		echo_client(fd);
	}

	--__left_fibers;
	printf("max_fibers: %d, left: %d\r\n", __max_fibers, __left_fibers);

	if (__left_fibers == 0) {
		double spent;

#if defined(_WIN32) || defined(_WIN64)
        DWORD end = GetTickCount();
        spent = (double)(end - __begin);
#else
		struct timeval end;
		gettimeofday(&end, NULL);
		spent = stamp_sub(&end, &__begin);
#endif

		printf("fibers: %d, clients: %d, error: %d, count: %lld, "
			"spent: %.2f ms, speed: %.2f tps\r\n", __max_fibers,
			__total_clients, __total_error_clients,
			__total_count, spent,
			(__total_count * 1000) / (spent > 0 ? spent : 1));
	}
}

static void fiber_main(ACL_FIBER *fiber, void *ctx)
{
	int i;

	(void) fiber;
	(void) ctx;

	for (i = 0; i < __max_fibers; i++) {
		acl_fiber_create(fiber_connect, NULL, __stack_size);
	}
}

static void usage(const char *procname)
{
	printf("usage: %s -h [help]\r\n"
		" -e event_mode [kernel|select|poll]\r\n"
		" -s server_ip\r\n"
		" -p server_port\r\n"
		" -t connt_timeout\r\n"
		" -r rw_timeout\r\n"
		" -c max_fibers\r\n"
		" -S [if using single IO, dafault: no]\r\n"
		" -z stack_size\r\n"
		" -n max_loop\r\n", procname);
}

int main(int argc, char *argv[])
{
	int   ch, event_mode = FIBER_EVENT_KERNEL;
       
	snprintf(__server_ip, sizeof(__server_ip), "%s", "127.0.0.1");

	socket_init();

	while ((ch = getopt(argc, argv, "hc:n:s:p:t:r:Sz:e:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'c':
			__max_fibers = atoi(optarg);
			__left_fibers = __max_fibers;
			break;
		case 't':
			__conn_timeout = atoi(optarg);
			break;
		case 'r':
			__rw_timeout = atoi(optarg);
			break;
		case 'n':
			__max_loop = atoi(optarg);
			break;
		case 's':
			snprintf(__server_ip, sizeof(__server_ip), "%s", optarg);
			break;
		case 'p':
			__server_port = atoi(optarg);
			break;
		case 'S':
			__read_data = 0;
			break;
		case 'z':
			__stack_size = atoi(optarg);
			break;
		case 'e':
			if (strcmp(optarg, "select") == 0)
				event_mode = FIBER_EVENT_SELECT;
			else if (strcmp(optarg, "poll") == 0)
				event_mode = FIBER_EVENT_POLL;
			else if (strcmp(optarg, "kernel") == 0)
				event_mode = FIBER_EVENT_KERNEL;
			break;
		default:
			break;
		}
	}

	acl_fiber_msg_stdout_enable(1);

#if defined(_WIN32) || defined(_WIN64)
    __begin = GetTickCount();
#else
	gettimeofday(&__begin, NULL);
#endif

	acl_fiber_create(fiber_main, NULL, __stack_size);

	printf("call fiber_schedule with=%d\r\n", event_mode);

	acl_fiber_schedule_with(event_mode);

#if defined(_WIN32) || defined(_WIN64)
	socket_end();
#endif

	return 0;
}
