# The high performance coroutine library, supporting Linux/BSD/Windows

## which IO events are supported ?
The libfiber supports many events including select/poll/epoll/kqueue/iocp, and Windows GUI messages.

Event|Linux|BSD|Windows
----------|------|---|---
<b>select</b>|yes|yes|yes
<b>poll</b>|yes|yes|yes
<b>epoll</b>|yes|no|no
<b>kqueue</b>|no|yes|no
<b>iocp</b>|no|no|yes
<b>Win GUI message</b>|no|no|yes

## SAMPLES

### One server sample
~~~C
// fiber_server.c

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fiber/lib_fiber.h"
#include "patch.h" // in the samples path

static size_t      __stack_size  = 128000;
static const char *__listen_ip   = "127.0.0.1";
static int         __listen_port = 9001;

static void fiber_client(ACL_FIBER *fb, void *ctx)
{
	SOCKET *pfd = (SOCKET *) ctx;
	char buf[8192];

	while (1) {
		int ret = acl_fiber_recv(*pfd, buf, sizeof(buf), 0);
		if (ret == 0) {
			break;
		} else if (ret < 0) {
			if (acl_fiber_last_error() == FIBER_EINTR) {
				continue;
			}
			break;
		}
		if (acl_fiber_send(*pfd, buf, ret, 0) < 0) {
			break;
		}
	}

	socket_close(*pfd);
	free(pfd);
}

static void fiber_accept(ACL_FIBER *fb, void *ctx)
{
	const char *addr = (const char *) ctx;
	SOCKET lfd = socket_listen(__listen_ip, __listen_port);

	assert(lfd >= 0);

	for (;;) {
		SOCKET *pfd, cfd = socket_accept(lfd);
		if (cfd == INVALID_SOCKET) {
			printf("accept error %s\r\n", acl_fiber_last_serror());
			break;
		}
		pfd = (SOCKET *) malloc(sizeof(SOCKET));
		*pfd = cfd;
		acl_fiber_create(fiber_client, pfd, __stack_size);
	}

	socket_close(lfd);
	exit (1);
}

// FIBER_EVENT_KERNEL represents the event type on
// Linux(epoll), BSD(kqueue), Windows(iocp)
// FIBER_EVENT_POLL: poll on Linux/BSD/Windows
// FIBER_EVENT_SELECT: select on Linux/BSD/Windows
// FIBER_EVENT_WMSG: Win GUI message on Windows
// acl_fiber_create/acl_fiber_schedule_with are in `lib_fiber.h`.
// socket_listen/socket_accept/socket_close are in patch.c of the samples path.

int main(void)
{
	int event_mode = FIBER_EVENT_KERNEL;

#if defined(_WIN32) || defined(_WIN64)
	socket_init();
#endif

	acl_fiber_create(fiber_accept, NULL, __stack_size);
	acl_fiber_schedule_with(event_mode);

#if defined(_WIN32) || defined(_WIN64)
	socket_end();
#endif

	return 0;
}
~~~

### One client sample

~~~C
// fiber_client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fiber/lib_fiber.h"
#include "patch.h" // in the samples path

static const char *__server_ip   = "127.0.0.1";
static int         __server_port = 9001;

// socket_init/socket_end/socket_connect/socket_close are in patch.c of the samples path

static void fiber_client(ACL_FIBER *fb, void *ctx)
{
	SOCKET cfd = socket_connect(__server_ip, __server_port);
	const char *s = "hello world\r\n";
	char buf[8192];
	int i, ret;

	if (cfd == INVALID_SOCKET) {
		return;
	}

	for (i = 0; i < 1024; i++) {
		if (acl_fiber_send(cfd, s, strlen(s), 0) <= 0) {
			printf("send error %s\r\n", acl_fiber_last_serror());
			break;
		}
		ret = acl_fiber_recv(cfd, buf, sizeof(buf), 0);
		if (ret <= 0) {
			break;
		}
	}

	acl_fiber_close(cfd);
}

int main(void)
{
	int event_mode = FIBER_EVENT_KERNEL;
	size_t stack_size = 128000;

	int i;

#if defined(_WIN32) || defined(_WIN64)
	socket_init();
#endif

	for (i = 0; i < 100; i++) {
		acl_fiber_create(fiber_client, NULL, stack_size);
	}

	acl_fiber_schedule_with(event_mode);

#if defined(_WIN32) || defined(_WIN64)
	socket_end();
#endif

	return 0;
}
~~~

## BUILDING
~~~
fiber_server: fiber_server.c
	gcc -o fiber_server fiber_server.c patch.c -I{path_of_fiber_header} -L{path_of_fiber_lib) -lfiber -ldl -lpthread

fiber_client: fiber_client.c
	gcc -o fiber_client fiber_client.c patch.c -I{path_of_fiber_header} -L{path_of_fiber_lib) -lfiber -ldl -lpthread
~~~
