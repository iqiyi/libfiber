#include "patch.h"
#if defined(_WIN32) || defined(_WIN64)
#include <signal.h>
#endif
#include <winsock2.h>

void socket_init(void)
{
#if defined(_WIN32) || defined(_WIN64)
	WORD version = 0;
	WSADATA data;

	FillMemory(&data, sizeof(WSADATA), 0);
	version = MAKEWORD(2, 0);

	if (WSAStartup(version, &data) != 0) {
		abort();
	}
#else
	signal(SIGPIPE, SIG_IGN);
#endif
}

void socket_end(void)
{
#if defined(_WIN32) || defined(_WIN64)
	WSACleanup();
#endif
}
