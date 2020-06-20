#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "util.h"

double stamp_sub(const struct timeval& from, const struct timeval& sub)
{
	struct timeval res;

	memcpy(&res, &from, sizeof(struct timeval));

	res.tv_usec -= sub.tv_usec;
	if (res.tv_usec < 0) {
		--res.tv_sec;
		res.tv_usec += 1000000;
	}

	res.tv_sec -= sub.tv_sec;
	return res.tv_sec * 1000.0 + res.tv_usec / 1000.0;
}

double compute_speed(long long n, double cost)
{
	if (cost <= 0) { // xxxx ?
		cost = 0.001;
	}
	double speed = (n * 1000) / cost;
	return speed;
}

void show_speed(const struct timeval& begin, const struct timeval& end,
	long long n)
{
	double cost = stamp_sub(end, begin);
	double speed = compute_speed(n, cost);
	printf(">>total count=%lld, cost=%.2f, speed=%.2f\r\n",
		n, cost, speed);
}
