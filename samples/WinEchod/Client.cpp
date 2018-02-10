#include "stdafx.h"
#include "Client.h"

CClient::CClient(socket_t sock)
	: m_sock(sock)
{
}

CClient::~CClient(void)
{
	acl_fiber_close(m_sock);
}

void CClient::Run(void)
{
	char buf[1024];
	int  n = 0;

	while (true)
	{
		int ret = acl_fiber_recv(m_sock, buf, sizeof(buf) - 1, 0);
		if (ret <= 0)
		{
			int err = acl_fiber_last_error();
			printf("recv error: %s, %d, n=%d\r\n",
				acl_fiber_last_serror(), err, n);
			break;
		}
		buf[ret] = 0;
		//printf("recv=%d, [%s]\r\n", ret, buf);
		if (acl_fiber_send(m_sock, buf, ret, 0) == -1)
		{
			int err = acl_fiber_last_error();
			printf("write error %s, %d, n = %d\r\n",
				acl_fiber_last_serror(), err, n);
			break;
		}
		n++;
	}
}
