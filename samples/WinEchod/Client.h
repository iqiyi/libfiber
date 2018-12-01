#pragma once

class CClient
{
public:
	CClient(socket_t sock);
	~CClient(void);

	void Run(void);

private:
	socket_t m_sock;
};
