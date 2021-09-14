#pragma once

class CEchoServerDlg;

class CFiberConnect
{
public:
	CFiberConnect(CEchoServerDlg& dlg, const CString& ip,
		UINT port, int ioCount);
	~CFiberConnect(void);

	bool Start(void);

private:
	CEchoServerDlg& m_dlg;
	CString m_ip;
	UINT    m_port;
	int     m_ioCount;
};

