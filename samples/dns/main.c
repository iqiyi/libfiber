#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "fiber/libfiber.h"

static void print_res(struct hostent *h_addrp)
{
	char **pptr;

	for (pptr = h_addrp->h_addr_list; *pptr != NULL; pptr++) {
		struct sockaddr_in sa;
		int n;

		if ((size_t) h_addrp->h_length > sizeof(sa.sin_addr)) {
			n = sizeof(sa.sin_addr);
		} else {
			n = h_addrp->h_length;
		}

		memcpy(&sa.sin_addr, *pptr, n);
		printf("  %s\r\n", inet_ntoa(sa.sin_addr));
	}
	printf("\r\n");
}

static void nslookup(ACL_FIBER *fiber, void *ctx)
{
	char  *name = (char *)ctx;
	struct hostent  h_buf;
	struct hostent *h_addrp;
	char   buf[1024];
	int    err;

	(void) fiber;

	if (gethostbyname_r(name, &h_buf, buf, sizeof(buf), &h_addrp, &err)) {
		printf("can't resolve: %s\r\n", name);
	} else if (h_addrp == NULL || h_addrp->h_addr_list == NULL) {
		printf("can't resolve: %s\r\n", name);
	} else {
		printf("%s: %d\r\n", name, h_addrp->h_length);
		fflush(stdout);
		print_res(h_addrp);
	}

	free(name);
}

int main(int argc, char *argv[])
{
	char  *ptr, *p;

	if (argc <=1) {
		printf("usage: %s addrs\r\n", argv[0]);
		return 1;
	}

	ptr = argv[1];
	p   = ptr;
	while ((ptr = strtok(p, ",; \t")) != NULL) {
		char *addr = strdup(ptr);
		acl_fiber_create(nslookup, addr, 32000);
		p = NULL;
	}

	acl_fiber_schedule();

	return 0;
}
