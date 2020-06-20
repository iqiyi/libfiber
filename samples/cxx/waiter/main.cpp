#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include <iostream>
#include <sstream>

#include "fiber/libfiber.hpp"
#include "fiber/go_fiber.hpp"

static void fiber_main(int i) {
	printf("fiber-%d, i=%d, thread is %ld\r\n",
		acl::fiber::self(), i, (long) pthread_self());

	// wait for thread running
	go_wait[&] {
		// run in another thread
		printf("i=%d, current thread is %ld\r\n", i, (long) pthread_self());
		i += 100;  // change the i's value
		usleep(10000);
	};

	// after thread return

	printf("ok, fiber-%d, i=%d, thread is %ld\r\n",
		acl::fiber::self(), i, (long) pthread_self());
}

static void test_run(void) {
	for (int i = 0; i < 10; i++) {
		go[=] { fiber_main(i); };
	}

	acl::fiber::schedule();
}

int main(void) {
	test_run();
	return 0;
}
