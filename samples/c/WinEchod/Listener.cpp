#include "stdafx.h"
#include "Client.h"
#include "Listener.h"

CListener::CListener(socket_t sock)
	: m_listenfd(sock)
{
}

CListener::~CListener(void)
{
	acl_fiber_close(m_listenfd);
}

static void fiber_client(ACL_FIBER* fb, void* ctx)
{
	CClient* conn = (CClient*) ctx;
	conn->Run();
	delete conn;
}

void CListener::Run(void)
{
	printf("listener fiber run ...\r\n");
	acl_fiber_delay(1000);
	printf("wakeup now\r\n");

	int n = 0;
	while (true)
	{
		socket_t sock = acl_fiber_accept(m_listenfd, NULL, NULL);
		if (sock == INVALID_SOCKET)
		{
			printf("accept error %s\r\n", acl_fiber_last_serror());
			break;
		}
		printf("accept one connection, sock=%d, n=%d\r\n", sock, ++n);
		CClient* conn = new CClient(sock);
		acl_fiber_create(fiber_client, conn, 128000);
	}

	printf("listening stopped!\r\n");
}
