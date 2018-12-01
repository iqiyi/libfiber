#pragma once

class CWinEchodDlg;

class CConnect
{
public:
	CConnect(CWinEchodDlg& hWin, const char* ip, int port, int count);
	~CConnect(void);

	void Run(void);

private:
	CWinEchodDlg& m_hWin;
	char m_serverIP[128];
	int  m_serverPort;
	int  m_count;

	void doEcho(socket_t sock);
};

