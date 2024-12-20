#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <iostream>
#include "fiber/wait_group.hpp"
#include "fiber/go_fiber.hpp"

static int __delay = 1;

static void usage(const char* procname) {
	printf("usage: %s -h [help]\r\n"
		" -e event_type[kernel|poll|select|io_uring]\r\n"
		" -t threads_count[default: 1]\r\n"
		" -c fibers_count[default: 1]\r\n"
		" -d delay[default: 1 second]\r\n"
		, procname);
}

int main(int argc, char *argv[]) {
	int  ch, threads_count = 1, fibers_count = 1;
	acl::fiber_event_t event_type = acl::FIBER_EVENT_T_KERNEL;

	while ((ch = getopt(argc, argv, "he:t:f:c:d:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'e':
			if (strcasecmp(optarg, "kernel") == 0) {
				event_type = acl::FIBER_EVENT_T_KERNEL;
			} else if (strcasecmp(optarg, "select") == 0) {
				event_type = acl::FIBER_EVENT_T_SELECT;
			} else if (strcasecmp(optarg, "poll") == 0) {
				event_type = acl::FIBER_EVENT_T_POLL;
			} else if (strcasecmp(optarg, "io_uring") == 0) {
				event_type = acl::FIBER_EVENT_T_IO_URING;
			}
			break;
		case 't':
			threads_count = atoi(optarg);
			break;
		case 'c':
			fibers_count = atoi(optarg);
			break;
		case 'd':
			__delay = atoi(optarg);
			break;
		default:
			break;
		}
	}

	acl::fiber::stdout_open(true);

	acl::wait_group sync;
	sync.add(threads_count + fibers_count);

	for (int i = 0; i < threads_count; i++) {
		std::thread* thr = new std::thread([&sync] {
			std::cout << "Thread " << std::this_thread::get_id()
				<< " started, delay " << __delay << " seconds\r\n";

			::sleep((unsigned) __delay);

			std::cout << "Thread "<< std::this_thread::get_id() << " done!\r\n";
			sync.done();
		});

		thr->detach();
	}

	for (int i = 0; i < fibers_count; i++) {
		go[&sync] {
			std::cout << "Fiber-" << acl::fiber::self()
				<< " started, delay " << __delay << " seconds\r\n",

			::sleep((unsigned) __delay);

			std::cout << "Fiber-" << acl::fiber::self() << " done!\r\n";
			sync.done();
		};
	}

	go[&sync] {
		sync.wait();
		printf("All threads and fibers were done\r\n");
	};

	acl::fiber::schedule_with(event_type);
	return 0;
}
