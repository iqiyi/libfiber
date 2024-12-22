#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <thread>
#include <iostream>

#include "fiber/fiber_tbox2.hpp"
#include "fiber/go_fiber.hpp"

class myobj {
public:
    myobj() {
        std::cout << "Thread: " << std::this_thread::get_id() << " myobj created!\r\n";
    }

    ~myobj() {
        std::cout << "Thread: " << std::this_thread::get_id() << " myobj freed!\r\n";
    }

    void run() {
        std::cout << "Thread: " << std::this_thread::get_id() << " hello world!\r\n";
    }
};

using shared_myobj = std::shared_ptr<myobj>;
using shared_tbox2 = std::shared_ptr<acl::fiber_tbox2<shared_myobj>>;

static void usage(const char* procname) {
    printf("usage: %s -h [help]\r\n", procname);
}

int main(int argc, char* argv[]) {
    int ch;

    while ((ch = getopt(argc, argv, "h")) > 0) {
        switch (ch) {
        case 'h':
            usage(argv[0]);
            return 0;
        default:
            break;
        }
    }

    shared_tbox2 box(new acl::fiber_tbox2<shared_myobj>);

    go[box] {
        shared_myobj o;
        box->pop(o);
        o->run();
    };

    go[box] {
        shared_myobj o(new myobj);
        box->push(o);
    };

    //////////////////////////////////////////////////////////////////////////

    go[box] {
        shared_myobj o(new myobj);
        box->push(o);
    };

    std::thread thread([box] {
        shared_myobj o;
        box->pop(o);
        o->run();
    });

    acl::fiber::schedule();
    thread.join();
    return 0;
}
