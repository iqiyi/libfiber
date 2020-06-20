#pragma once

#include <sys/time.h>

double stamp_sub(const struct timeval& from, const struct timeval& sub);
void show_speed(const struct timeval& begin, const struct timeval& end, long long n);
double compute_speed(long long n, double cost);
