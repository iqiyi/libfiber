#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fiber/go_fiber.hpp"

int main() {
	std::shared_ptr<int> count(new int);

	for (int i = 0; i < 10; i++) {
		go_share(4096)[count] {
			printf("fiber-%d running\r\n", acl::fiber::self());
			(*count)++;
		};
	}

	acl::fiber::schedule();

	printf("At last, count is %d\r\n", *count);
	return 0;
}
