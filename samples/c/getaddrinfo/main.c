#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "fiber/libfiber.h"

static void show(const char *name, const struct addrinfo *res) {
	printf("name=%s, canonname=%s\r\n", name, res->ai_canonname);

	for (; res != NULL; res = res->ai_next) {
		char ip[128];
		const void *addr;

		if (res->ai_family == PF_INET) {
			const struct sockaddr_in *in =
				(const struct sockaddr_in *) res->ai_addr;
			addr = &in->sin_addr;

		} else if (res->ai_family == PF_INET6) {
			const struct sockaddr_in6 *in6 =
				(const struct sockaddr_in6 *) res->ai_addr;
			addr = &in6->sin6_addr;
		} else {
			continue;
		}

		if (inet_ntop(res->ai_family, addr, ip, sizeof(ip))) {
			printf("    ip=%s\r\n", ip);
		}
	}
}

static void lookup(ACL_FIBER *fiber, void *ctx) {
	char *name = (char *) ctx;
	struct addrinfo hints, *res;
	int ret;

	(void) fiber; // avoid compiler warning

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;

	ret = getaddrinfo(name, "80", &hints, &res);

	if (ret != 0) {
		printf("getaddrinfo error %s, name=%s\r\n",
			gai_strerror(ret), name);
	} else {
		show(name, res);
		freeaddrinfo(res);
	}
	free(name);
}

int main(void) {
	char *name1 = strdup("www.iqiyi.com");
	char *name2 = strdup("www.baidu.com");

	acl_fiber_create(lookup, name1, 128000);
	acl_fiber_create(lookup, name2, 128000);

	acl_fiber_schedule();
	return 0;
}
