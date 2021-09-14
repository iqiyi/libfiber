#pragma once

class CFiberServer : public acl::fiber
{
public:
	CFiberServer(bool autoDestroy = true);
	~CFiberServer(void);

	bool BindAndListen(UINT port, const CString& addr);

protected:
	// @override
	void run(void);

private:
	SOCKET m_sock;
	bool   m_autoDestroy;
};

