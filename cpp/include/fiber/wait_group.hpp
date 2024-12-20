#pragma once
#include "fiber_cpp_define.hpp"

namespace acl {

template<typename T> class fiber_tbox;

class FIBER_CPP_API wait_group {
public:
	wait_group();
	~wait_group();

	void add(int n);
	void done();
	void wait();

private:
	void* state_;
	long long n_;
	fiber_tbox<unsigned long>* box_;
};

} // namespace acl
