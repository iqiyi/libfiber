#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <fiber/libfiber.h>
#include "../patch.h"

static int  __nconnect = 0;
static int  __count = 0;
static int  __socket_count = 0;
static char __listen_ip[64];
static int  __listen_port = 9001;
static int  __listen_qlen = 64;
static int  __rw_timeout = 0;
static int  __echo_data  = 1;
static int  __stack_size = 128000;

static int check_read(int fd, int timeout)
{
	struct pollfd pfd;
	int n;

	memset(&pfd, 0, sizeof(struct pollfd));
	pfd.fd = fd;
	pfd.events = POLLIN;

	n = acl_fiber_poll(&pfd, 1, timeout);
	if (n < 0) {
		printf("poll error: %s\r\n", strerror(errno));
		return -1;
	}

	if (n == 0)
		return 0;
	if (pfd.revents & POLLIN)
		return 1;
	else
		return 0;
}

static void echo_client(ACL_FIBER *fiber, void *ctx)
{
	SOCKET  *pfd = (SOCKET *) ctx;
	SOCKET   fd  = *pfd;
	char  buf[8192];
	int   ret;

	(void) fiber;
	__socket_count++;
	printf("client fiber-%d: fd: %d\r\n", acl_fiber_self(), fd);

	while (1) {
		if (__rw_timeout > 0) {
			ret = check_read(fd, __rw_timeout * 1000);
			if (ret < 0)
				break;
			if (ret == 0) {
				printf("read timeout fd=%u\r\n", fd);
				break;
			}
		}

		ret = acl_fiber_recv(fd, buf, sizeof(buf) - 1, 0);
		if (ret == 0) {
			printf("read close by peer fd: %d, %s\r\n",
				fd, strerror(errno));
			break;
		} else if (ret < 0) {
			if (errno == EINTR) {
				printf("catch a EINTR signal\r\n");
				continue;
			}

			printf("read error %s, fd: %u\n", strerror(errno), fd);
			break;
		}

		// buf[ret] = 0; printf("buf=%s\r\n", buf);
		__count++;

		if (!__echo_data)
			continue;

		if (acl_fiber_send(fd, buf, ret, 0) < 0) {
			if (errno == EINTR)
				continue;
			printf("write error, fd: %d\r\n", fd);
			break;
		}
	}

	__socket_count--;
	printf("%s: close %d, socket_count=%d\r\n",
		__FUNCTION__, fd, __socket_count);
	socket_close(fd);
	free(pfd);

	if (--__nconnect == 0) {
		printf("\r\n----total read/write: %d----\r\n", __count);
		__count = 0;
	}
}

static void fiber_accept(ACL_FIBER *fiber, void *ctx)
{
	SOCKET lfd = socket_listen(__listen_ip, __listen_port);

	(void) fiber;
	(void) ctx;

	printf("fiber-%d listen %s:%d ok\r\n",
		acl_fiber_self(), __listen_ip, __listen_port);

	for (;;) {
		SOCKET *pfd;
		SOCKET  cfd = socket_accept(lfd);
		if (cfd == INVALID_SOCKET) {
			printf("accept error %s\r\n", strerror(errno));
			break;
		}

		pfd = malloc(sizeof(SOCKET));
		assert(pfd != NULL);
		*pfd = cfd;

		__nconnect++;
		printf("accept one, fd: %u, %p\r\n", cfd, pfd);
		acl_fiber_create(echo_client, pfd, __stack_size);
	}

	socket_close(lfd);
	exit(0);
}

//#define SCHEDULE_AUTO

#ifndef	SCHEDULE_AUTO
static void fiber_memcheck(ACL_FIBER *fiber, void *ctx)
{
	(void) fiber;
	(void) ctx;

	while (1) {
		acl_fiber_delay(1000);
	}
}
#endif

static void usage(const char *procname)
{
	printf("usage: %s -h [help]\r\n"
		" -e event_mode [kernel|select|poll]\r\n"
		" -s listen_ip\r\n"
		" -p listen_port\r\n"
		" -r rw_timeout\r\n"
		" -q listen_queue\r\n"
		" -z stack_size\r\n"
		" -S [if using single IO, default: no]\r\n", procname);
}

int main(int argc, char *argv[])
{
	int   ch, event_mode = FIBER_EVENT_KERNEL;

	snprintf(__listen_ip, sizeof(__listen_ip), "%s", "127.0.0.1");

	socket_init();

	while ((ch = getopt(argc, argv, "hs:p:r:q:Sz:e:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			snprintf(__listen_ip, sizeof(__listen_ip), "%s", optarg);
			break;
		case 'p':
			__listen_port = atoi(optarg);
			break;
		case 'r':
			__rw_timeout = atoi(optarg);
			break;
		case 'q':
			__listen_qlen = atoi(optarg);
			break;
		case 'S':
			__echo_data = 0;
			break;
		case 'z':
			__stack_size = atoi(optarg);
			break;
		case 'e':
			if (strcmp(optarg, "select") == 0)
				event_mode = FIBER_EVENT_SELECT;
			else if (strcmp(optarg, "poll") == 0)
				event_mode = FIBER_EVENT_POLL;
			break;
		default:
			break;
		}
	}

	acl_fiber_msg_stdout_enable(1);

#ifdef	SCHEDULE_AUTO
	acl_fiber_schedule_init(1);
	acl_fiber_schedule_set_event(event_mode);
#endif

	printf("%s: call fiber_creater\r\n", __FUNCTION__);
	acl_fiber_create(fiber_accept, NULL, __stack_size);

#ifndef	SCHEDULE_AUTO
	acl_fiber_create(fiber_memcheck, NULL, __stack_size);

	printf("call fiber_schedule\r\n");
	acl_fiber_schedule_with(event_mode);
#endif

	socket_end();

	return 0;
}
