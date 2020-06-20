#pragma once

class CListener
{
public:
	CListener(socket_t sock);
	~CListener(void);

	void Run(void);

private:
	socket_t m_listenfd;
};

