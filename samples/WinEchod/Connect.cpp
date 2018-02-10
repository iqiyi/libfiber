#include "stdafx.h"
#include "Resource.h"
#include "WinEchodDlg.h"
#include "afxdialogex.h"
#include "Connect.h"

CConnect::CConnect(CWinEchodDlg& hWin, const char* serverIP,
		int serverPort, int count)
	: m_hWin(hWin)
	, m_serverPort(serverPort)
	, m_count(count)
{
	size_t len = strlen(serverIP);
	if (len > sizeof(m_serverIP) - 1)
		len = sizeof(m_serverIP) - 1;
	memcpy(m_serverIP, serverIP, len);
	m_serverIP[len] = 0;
}

CConnect::~CConnect(void)
{
}

void CConnect::Run(void)
{
	socket_t conn = socket_connect(m_serverIP, m_serverPort);

	if (conn == INVALID_SOCKET)
		printf("connect %s:%d error %s\r\n",
			m_serverIP, m_serverPort, acl_fiber_last_serror());
	else
	{
		doEcho(conn);
		acl_fiber_close(conn);
	}

	m_hWin.OnFiberConnectExit();
}

void CConnect::doEcho(socket_t sock)
{
	char buf[1024];
	const char* s = "hello world\r\n";
	int i;
	for (i = 0; i < m_count; i++)
	{
		if (acl_fiber_send(sock, s, strlen(s), 0) < 0)
		{
			printf("send error %s\r\n", acl_fiber_last_serror());
			break;
		}
		int n = acl_fiber_recv(sock, buf, sizeof(buf) - 1, 0);
		if (n <= 0)
		{
			printf("read error %s\r\n", acl_fiber_last_serror());
			break;
		}
		//buf[n] = 0;
		//printf("%s\r\n", buf);
	}
	printf("Echo over, fd=%u, i=%d\r\n", sock, i);
}
