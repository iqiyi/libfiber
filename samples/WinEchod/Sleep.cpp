#include "stdafx.h"
#include "Sleep.h"

CSleep::CSleep(void)
{
}

CSleep::~CSleep(void)
{
}

void CSleep::Run(void)
{
	printf("timer fiber-%d created\r\n", acl_fiber_self());
	for (int i = 0; i < 5; i++)
	{
		acl_fiber_delay(1000);
		printf("fiber-%d wakeup\r\n", acl_fiber_self());
	}

	printf("sleep fiber exit now\r\n");
}
