#include "pch.h"
#include "EchoServerDlg.h"
#include "FiberConnect.h"

CFiberConnect::CFiberConnect(CEchoServerDlg& dlg, const CString& ip,
	UINT port, int ioCount)
: m_dlg(dlg)
, m_ip(ip)
, m_port(port)
, m_ioCount(ioCount)
{
}

CFiberConnect::~CFiberConnect(void)
{
}

bool CFiberConnect::Start(void)
{
	struct sockaddr_in in;
	in.sin_family = AF_INET;
	in.sin_port = htons(m_port);
	if (inet_pton(AF_INET, m_ip.GetString(), &in.sin_addr) == -1) {
		return false;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		return false;
	}

	if (connect(sock, (const sockaddr*)&in, sizeof(in)) == -1) {
		closesocket(sock);
		return false;
	}

	const char data[] = "hello world!\r\n";
	char buf[256];
	int i;
	for (i = 0; i < m_ioCount; i++) {
		int ret = send(sock, data, sizeof(data) - 1, 0);
		if (ret == -1) {
			break;
		}
		ret = recv(sock, buf, sizeof(buf), 0);
		if (ret <= 0) {
			break;
		}
		m_dlg.OnFiberIo();
	}

	closesocket(sock);
	m_dlg.OnFiberConnectDone();
	return true;
}
