#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <thread>
#include <iostream>

#include "fiber/fiber_tbox.hpp"
#include "fiber/go_fiber.hpp"

class myobj {
public:
    myobj() = default;
    ~myobj() = default;

    void run() {
        std::cout << "Thread: " << std::this_thread::get_id() << " hello world!\r\n";
    }
};

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

    std::shared_ptr<acl::fiber_tbox<myobj>> box(new acl::fiber_tbox<myobj>);

    go[box] {
        myobj *o = box->pop();
        o->run();
        delete o;
    };

    go[box] {
        myobj *o = new myobj;
        box->push(o);
    };

    //////////////////////////////////////////////////////////////////////////

    go[box] {
        myobj *o = new myobj;
        box->push(o);
    };

    std::thread thread([box] {
        myobj *o = box->pop();
        o->run();
        delete o;
    });

    acl::fiber::schedule();
    thread.join();
    return 0;
}
