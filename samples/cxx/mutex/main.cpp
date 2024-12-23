#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <iostream>

#include "fiber/go_fiber.hpp"

int main() {
	acl::fiber_mutex mutex;

	for (int j = 0; j < 5; j++) {
		go[&mutex] {
			for (int i = 0; i < 5; i++) {
				mutex.lock();
				printf("fiber-%d: locked\r\n", acl::fiber::self());
				::sleep(1);
				mutex.unlock();
				acl::fiber::yield();
			}
		};
	}

	std::vector<std::thread> threads;

	for (int i = 0; i < 5; i++) {
		threads.emplace_back(std::thread([&mutex] {
			for (int j = 0; j < 5; j++) {
				mutex.lock();
				std::cout << "Thread: " << std::this_thread::get_id() << " locked\r\n";
				::sleep(1);
				mutex.unlock();
				::sleep(1);
			}
		}));
	}

	acl::fiber::schedule_with(acl::FIBER_EVENT_T_KERNEL);

	for (auto& thr : threads) {
		thr.join();
	}

	return 0;
}
