#include "stdafx.hpp"
#include <assert.h>
#include "../../c/src/common/atomic.h"
#include "fiber/fiber_tbox.hpp"
#include "fiber/wait_group.hpp"

namespace acl {

wait_group::wait_group()
{
	box_   = new acl::fiber_tbox<unsigned long>;
	state_ = (void*) atomic_new();
	n_     = 0;

	atomic_set((ATOMIC*) state_, &n_);
	atomic_int64_set((ATOMIC*) state_, n_);
}

wait_group::~wait_group()
{
	delete box_;
	atomic_free((ATOMIC*) state_);
}

void wait_group::add(int n)
{
	long long state = atomic_int64_add_fetch((ATOMIC*) state_,
			(long long) n << 32);

	// The high 32 bits store the number of tasks.
	int c = (int)(state >> 32);

	// The low 32 bits store the number of waiters.
	unsigned w =  (unsigned) state;

	// count must be >= 0.
	if (c < 0){
		msg_fatal("Negative wait_group counter");
	}

	if (w != 0 && n > 0 && c == n){
		msg_fatal("Add called concurrently with wait");
	}

	if (c > 0 || w == 0) {
		return;
	}

	// Check if the state has been changed.
	if (atomic_int64_fetch_add((ATOMIC*) state_, 0) != state) {
		msg_fatal("Add called concurrently with wait");
	}

	// The count should be zero, so clear state and wakeup all the waiters.
	atomic_int64_set((ATOMIC*) state_, 0);

	for (size_t i = 0; i < w; i++) {
		box_->push(NULL);
	}
}

void wait_group::done()
{
	add(-1);
}

void wait_group::wait()
{
	for(;;) {
		long long state = atomic_int64_fetch_add((ATOMIC*) state_, 0);
		int c = (int) (state >> 32);

		// Return if no tasks.
		if (c == 0) {
			return;
		}

		// Increase the number of waiters, or get state again if failed.
		if (atomic_int64_cas((ATOMIC*) state_, state, state + 1) == state) {
			bool found;
			(void) box_->pop(-1, &found);
			assert(found);
			if(atomic_int64_fetch_add((ATOMIC*) state_, 0) == 0) {
				return;
			}

			msg_fatal("Reused before previous wait has returned");
		}
	}
}

} // namespace acl
