#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "fiber/libfiber.hpp"

class myfiber : public acl::fiber {
public:
	myfiber(int i) : i_(i) {}

private:
	int i_;
	acl::fiber_tbox<int> box_;

	~myfiber(void) {}

protected:
	// @override
	void run(void) {
		printf("fiber-%d, i=%d, thread is %ld\r\n",
			acl::fiber::self(), i_, (long) pthread_self());

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, 1);

		pthread_t tid;
		pthread_create(&tid, &attr, thread_main, this);

		// wait for message from thread
		(void) box_.pop();

		// after thread return
		printf("ok, fiber-%d, i=%d, thread is %ld\r\n",
			acl::fiber::self(), i_, (long) pthread_self());

		delete this;
	}

private:
	static void* thread_main(void* ctx) {
		myfiber* fb = (myfiber*) ctx;

		// run in another thread
		printf("i=%d, current thread is %ld\r\n",
			fb->i_, (long) pthread_self());
		fb->i_ += 100;  // change the i's value
		usleep(10000);

		// notify the waiter
		fb->box_.push(NULL);

		return NULL;
	}
};

static void test_run(void) {
	for (int i = 0; i < 10; i++) {
		acl::fiber* fb = new myfiber(i);
		fb->start();
	}

	acl::fiber::schedule();
}

int main(void) {
	test_run();
	return 0;
}
