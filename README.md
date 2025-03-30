# The high performance coroutine library, supporting Linux/BSD/Mac/Windows

[中文说明](README_cn.md)  

<!-- vim-markdown-toc GFM -->

* [1 About](#1-about)
* [2 Which IO events are supported?](#2-which-io-events-are-supported-)
* [3 SAMPLES](#3-samples)
    * [3.1 One server sample with C API](#31-one-server-sample-with-c-api)
    * [3.2 One client sample with C API](#32-one-client-sample-with-c-api)
    * [3.3 Resolve domain address in coroutine](#33-resolve-domain-address-in-coroutine)
    * [3.4 Create fiber with standard C++ API](#34-create-fiber-with-standard-c-api)
    * [3.5 Create fiber with C++1x API](#35-create-fiber-with-c1x-api)
    * [3.6 Create shared stack fiber](#36-create-shared-stack-fiber)
    * [3.7 Sync between fibers and threads](#37-sync-between-fibers-and-threads)
    * [3.8 Transfer objects through box](#38-transfer-objects-through-box)
    * [3.9 Using wait_group to wait for the others done](#39-using-wait_group-to-wait-for-the-others-done)
    * [3.10 Using fiber_pool to execute different tasks](#310-using-fiber_pool-to-execute-different-tasks)
    * [3.11 Wait for the result from a thread](#311-wait-for-the-result-from-a-thread)
    * [3.12 Http server supporting http url route](#312-http-server-supporting-http-url-route)
    * [3.13 Windows GUI sample](#313-windows-gui-sample)
    * [3.14 More SAMPLES](#314-more-samples)
* [4 BUILDING](#4-building)
    * [4.1 On Unix](#41-on-unix)
    * [4.2 On Windows](#42-on-windows)
* [5 Benchmark](#5-benchmark)
* [6 API support](#6-api-support)
    * [6.1 Base API](#61-base-api)
    * [6.2 IO API](#62-io-api)
    * [6.3 Net API](#63-net-api)
    * [6.4 Channel API](#64-channel-api)
    * [6.5 Sync API](#65-sync-api)
* [7 About API Hook](#7-about-api-hook)
* [8 FAQ](#8-faq)

<!-- vim-markdown-toc -->

## 1 About

The libfiber project comes from the coroutine module of the [acl project](#https://github.com/acl-dev/acl) in lib_fiber directory of which. It can be used on OS platforms including Linux, FreeBSD, macOS, and Windows, which supports select, poll, epoll, kqueue, iocp, and even Windows GUI messages for different platform. With libfiber, you can write network application services having the high performance and large concurrent more easily than the traditional asynchronous  framework with event-driven model. <b>What's more</b>, with the help of libfiber, you can even write network module of the Windows GUI application written by MFC, wtl or other GUI framework on Windows in coroutine way. That's really amazing.

## 2 Which IO events are supported ?

The libfiber supports many events including select/poll/epoll/kqueue/iocp, and Windows GUI messages.

| Platform       | Event type                      |
|----------------|---------------------------------|
| <b>Linux</b>   | select, poll, epoll, io-uring   |
| <b>BSD</b>     | select, poll, kqueue            |
| <b>Mac</b>     | select, poll, kqueue            |
| <b>Windows</b> | select, poll, iocp, GUI Message |

## 3 SAMPLES

### 3.1 One server sample with C API

```C
// fiber_server.c

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fiber/lib_fiber.h"
#include "patch.h" // in the samples' path

static size_t      __stack_size  = 128000;
static const char *__listen_ip   = "127.0.0.1";
static int         __listen_port = 9001;

// Set read/write timeout with setsockopt API.
static int set_timeout(SOCKET fd, int opt, int timeo) {
# if defined(_WIN32) || defined(_WIN64)
    timeout *= 1000; // From seconds to millisecond.
    if (setsockopt(fd, SOL_SOCKET, opt, (const char*) &timeo, sizeof(timeo)) < 0) {
        printf("setsockopt error=%s, timeout=%d, opt=%d, fd=%d\r\n",
            strerror(errno), timeo, opt, (int) fd);
		return -1;
	}
# else   // Must be Linux or __APPLE__.
    struct timeval tm;
    tm.tv_sec  = timeo;
    tm.tv_usec = 0;

    if (setsockopt(fd, SOL_SOCKET, opt, &tm, sizeof(tm)) < 0) {
        printf("setsockopt error=%s, timeout=%d, opt=%d, fd=%d\r\n",
            strerror(errno), timeo, opt, (int) fd);
        return -1;
    }
# endif
    return 0;
}

static int set_rw_timeout(SOCKET fd, int timeo) {
    if (set_timeout(fd, SO_RCVTIMEO, timeo) == -1 
       || set_timeout(fd, SO_SNDTIMEO, timeo) == -1) {
        return -1;
    }
    return 0;
}

static void fiber_client(ACL_FIBER *fb, void *ctx) {
    SOCKET *pfd = (SOCKET *) ctx;
    char buf[8192];

    // Set the socket's read/write timeout.
    set_rw_timeout(*pfd, 10);

    while (1) {
#ifdef _WIN32
        int ret = acl_fiber_recv(*pfd, buf, sizeof(buf), 0);
#else
        int ret = recv(*pfd, buf, sizeof(buf), 0);
#endif
        if (ret == 0) {
            break;
        } else if (ret < 0) {
            if (acl_fiber_last_error() == FIBER_EINTR) {
                continue;
            }
            break;
        }
#ifdef _WIN32
        if (acl_fiber_send(*pfd, buf, ret, 0) < 0) {
#else
        if (send(*pfd, buf, ret, 0) < 0) {
#endif			
            break;
        }
    }

    socket_close(*pfd);
    free(pfd);
}

static void fiber_accept(ACL_FIBER *fb, void *ctx) {
    const char *addr = (const char *) ctx;
    SOCKET lfd = socket_listen(__listen_ip, __listen_port);

    assert(lfd >= 0);

    for (;;) {
        SOCKET *pfd, cfd = socket_accept(lfd);
        if (cfd == INVALID_SOCKET) {
            printf("accept error %s\r\n", acl_fiber_last_serror());
            break;
        }
        pfd = (SOCKET *) malloc(sizeof(SOCKET));
        *pfd = cfd;

        // create and start one fiber to handle the client socket IO
        acl_fiber_create(fiber_client, pfd, __stack_size);
    }

    socket_close(lfd);
    exit (0);
}

// FIBER_EVENT_KERNEL represents the event type on
// Linux(epoll), BSD(kqueue), Mac(kqueue), Windows(iocp)
// FIBER_EVENT_POLL: poll on Linux/BSD/Mac/Windows
// FIBER_EVENT_SELECT: select on Linux/BSD/Mac/Windows
// FIBER_EVENT_WMSG: Win GUI message on Windows
// acl_fiber_create/acl_fiber_schedule_with are in `lib_fiber.h`.
// socket_listen/socket_accept/socket_close are in patch.c of the samples' path.

int main(void) {
    int event_mode = FIBER_EVENT_KERNEL;

#ifdef _WIN32
    socket_init();
#endif

    // create one fiber to accept connections
    acl_fiber_create(fiber_accept, NULL, __stack_size);

    // start the fiber schedule process
    acl_fiber_schedule_with(event_mode);

#ifdef _WIN32
    socket_end();
#endif

    return 0;
}
```

### 3.2 One client sample with C API

```C
// fiber_client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fiber/lib_fiber.h"
#include "patch.h" // in the samples' path

static const char *__server_ip   = "127.0.0.1";
static int         __server_port = 9001;

// socket_init/socket_end/socket_connect/socket_close are in patch.c of the samples path

static void fiber_client(ACL_FIBER *fb, void *ctx) {
    SOCKET cfd = socket_connect(__server_ip, __server_port);
    const char *s = "hello world\r\n";
    char buf[8192];
    int i, ret;

    if (cfd == INVALID_SOCKET) {
        return;
    }

    for (i = 0; i < 1024; i++) {
#ifdef _WIN32
        if (acl_fiber_send(cfd, s, strlen(s), 0) <= 0) {
#else
        if (send(cfd, s, strlen(s), 0) <= 0) {
#endif			
            printf("send error %s\r\n", acl_fiber_last_serror());
            break;
        }

#ifdef _WIN32
        ret = acl_fiber_recv(cfd, buf, sizeof(buf), 0);
#else
        ret = recv(cfd, buf, sizeof(buf), 0);
#endif		
        if (ret <= 0) {
            break;
        }
    }

#ifdef _WIN32
    acl_fiber_close(cfd);
#else
    close(cfd);
#endif
}

int main(void) {
    int event_mode = FIBER_EVENT_KERNEL;
    size_t stack_size = 128000;

    int i;

#ifdef _WIN32
    socket_init();
#endif

    for (i = 0; i < 100; i++) {
        acl_fiber_create(fiber_client, NULL, stack_size);
    }

    acl_fiber_schedule_with(event_mode);

#ifdef _WIN32
    socket_end();
#endif

    return 0;
}
```

### 3.3 Resolve domain address in coroutine

The rfc1035 for DNS has been implement in libfiber, so you can call gethostbyname or getaddrinfo to get the givent domain's IP addresses in coroutine.
```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include "fiber/lib_fiber.h"

static void lookup(ACL_FIBER *fiber, void *ctx) {
    char *name = (char *) ctx;
    struct addrinfo hints, *res0;
    int ret;

    (void) fiber; // avoid compiler warning

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;

    ret = getaddrinfo(name, "80", &hints, &res0);
    free(name);

    if (ret != 0) {
        printf("getaddrinfo error %s\r\n", gai_strerror(ret));
    } else {
        printf("getaddrinfo ok\r\n");
        freeaddrinfo(res0);
    }
}

int main(void) {
    char *name1 = strdup("www.iqiyi.com");
    char *name2 = strdup("www.baidu.com");

    acl_fiber_create(lookup, name1, 128000);
    acl_fiber_create(lookup, name2, 128000);

    acl_fiber_schedule();
    return 0;
}
```

### 3.4 Create fiber with standard C++ API

You can create one coroutine with standard C++ API in libfiber:
```C
#include <cstdio>
#include "fiber/libfiber.hpp"

class myfiber : public acl::fiber {
public:
    myfiber() {}

private:
    ~myfiber() {}

protected:
    // @override from acl::fiber
    void run() {
        printf("hello world!\r\n");
        delete this;
    }
};

int main() {
    for (int i = 0; i < 10; i++) {
        acl::fiber* fb = new myfiber();
        fb->start();
    }

    acl::fiber::schedule();
    return 0;
}
```

### 3.5 Create fiber with C++1x API

You can also create one coroutine with c++11 API in libfiber:
```C
#include <cstdio>
#include "fiber/libfiber.hpp"
#include "fiber/go_fiber.hpp"

static void fiber_routine(int i) {
    printf("hi, i=%d, curr fiber=%u\r\n", i, acl::fiber::self());
}

int main() {
    for (int i = 0; i < 10; i++) {
        go[=] {
            fiber_routine(i);
        };
    }

    acl::fiber::schedule();
    return 0;
}
```

### 3.6 Create shared stack fiber

You can create fiber in shared stack mode to decrease the memory's size.
```c++
#include <cstdio>
#include <cstdlib>
#include <memory>
#include "fiber/go_fiber.hpp"

void test_shared_stack() {
   std::shared_ptr<int> count(new int);
   for (int i = 0; i < 10; i++) {
       go_share(1024)[count] {
           (*count)++;
       }; 
   }
   
   acl::fiber::schedule();
   printf("At last the count is %d\r\n", *count);
}

```

### 3.7 Sync between fibers and threads

fiber_mutex can be used to sync between different fibers and threads:
```c++
#include <thread>
#include "fiber/go_fiber.hpp"
#include "fiber/fiber_mutex.hpp"

void test_mutex() {
    // Create one fiber mutex can be shared between different fibers and threads.
    std::shared_ptr<acl::fiber_mutex> mutex(new acl::fiber_mutex);
    
    // Create one fiber to use fiber mutex.
    go[mutex] {
        mutex->lock();
        ::sleep(1);
        mutex->unlock();
    };

    // Create one thread to use fiber mutex.
    std::thread([mutex] {
        mutex->lock();
        ::sleep(1);
        mutex->unlock();
    }).detach();

    // Create one thread and one fiber in it to use fiber mutex.
    std::thread([mutex] {
        go[mutex] {
            mutex->lock();
            ::sleep(1);
            mutex->unlock();
        };
        acl::fiber::schedule();
    }).detach();
    
    // Start the current thread's schedule.
    acl::fiber::schedule();
}
```

### 3.8 Transfer objects through box

You can use fiber_tbox or fiber_tbox2 to transfer objs between different fibers and threads:
```c++
#include <memory>
#include <thread>
#include "fiber/fiber_tbox.hpp"

class myobj {
public:
    myobj() = default;
    ~myobj() = default;
    void run() { printf("hello world!\r\n"); }
};

void test_tbox() {
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
    
    go[box] {
        myobj *o = box->pop();
        o->run();
        delete o;
    };
    std::thread thread([box] {
        myobj *o = new myobj;
        box->push(o);
    });
    thread.detach();
}
```

### 3.9 Using wait_group to wait for the others done

You can use wait_group to wait for the other tasks:
```c++
#include "fiber/go_fiber.hpp"
#include "fiber/wait_group.hpp"

void wait_others() {
    acl::wait_group wg;
    wg.add(2);

    std::thread thr([&wg]{
        ::sleep(1);
        wg.done();
    });
    thr.detach();

    go[&wg] {
        ::sleep(1);
        wg.done();
    };
    
    wg.wait();
}
```

### 3.10 Using fiber_pool to execute different tasks

You can use fiber_pool to run multiple tasks with high performance:
```c++
#include <cstdio>                                                             
#include <cstdlib>                                                             
#include <acl-lib/acl_cpp/lib_acl.hpp>                                         
#include <acl-lib/fiber/libfiber.hpp>

static void mytest(acl::wait_group& sync, int i) {
   printf("Task %d is running\n", i);
   sync.done();
}

int main() {
    // Create one fiber pool with min=1, max=20, idle=30s, buf=500, stack=64000.
    acl::fiber_pool pool(1, 20, 30, 500, 64000, false);
    acl::wait_group sync;
    int i = 0;
                                                                               
    sync.add(1);
    // Run the first task.
    pool.exec([&sync, i]() {
        printf("Task %d is running\n", i);
        sync.done();
    });
    i++;
                                                                               
    sync.add(1);
    // Run the second task.
    pool.exec([&sync](int i) {
       printf("Task %d is running\n", i);
       sync.done();
    }, i);
    i++;

    sync.add(1);
    // Run the third task.
    pool.exec(mytest, std::ref(sync), i);

    // Create one fiber to wait for all tasks done and stop the fiber pool.
    go[&sync, &pool] {
        sync.wait();
        pool.stop();
    };

    acl::fiber::schedule();
    return 0;
 }
```

### 3.11 Wait for the result from a thread

```C
#include <cstdio>
#include <unistd.h>
#include "fiber/go_fiber.hpp"

static void fiber_routine(int i) {
    go_wait[&] {	// running in another thread
        i += 100;
        ::usleep(10000);
    };

    printf("i is %d\r\n", i);
}

int main() {
    // create ten fibers
    for (int i = 0; i < 10; i++) {
        go[=] {
            fiber_routine(i);
        };
    }

    acl::fiber::schedule();
    return 0;
}
```

### 3.12 Http server supporting http url route

One http server written with libfiber and http module of [acl](https://github.com/acl-dev/acl) supports http handler route which is in [http server](https://github.com/acl-dev/acl/tree/master/lib_fiber/samples-c%2B%2B1x/httpd).

```C
#include <acl-lib/acl_cpp/lib_acl.hpp>          // must before http_server.hpp
#include <acl-lib/fiber/http_server.hpp>

static char *var_cfg_debug_msg;
static acl::master_str_tbl var_conf_str_tab[] = {
        { "debug_msg", "test_msg", &var_cfg_debug_msg },
        { 0, 0, 0 }
};

static int  var_cfg_io_timeout;
static acl::master_int_tbl var_conf_int_tab[] = {
        { "io_timeout", 120, &var_cfg_io_timeout, 0, 0 },
        { 0, 0 , 0 , 0, 0 }
};

int main(d) {
    acl::acl_cpp_init();
    acl::http_server server;

    // set the configure variables
    server.set_cfg_int(var_conf_int_tab)
          .set_cfg_str(var_conf_str_tab);

    // set http handler route
    server.Get("/", [](acl::HttpRequest&, acl::HttpResponse& res) {
        acl::string buf("hello world1!\r\n");
        res.setContentLength(buf.size());
        return res.write(buf.c_str(), buf.size());
    }).Post("/ok", [](acl::HttpRequest& req, acl::HttpResponse& res) {
        acl::string buf;
        req.getBody(buf);
        res.setContentLength(buf.size());
        return res.write(buf.c_str(), buf.size());
    }).Get("/json", [&](acl::HttpRequest&, acl::HttpResponse& res) {
        acl::json json;
        acl::json_node& root = json.get_root();
        root.add_number("code", 200)
            .add_text("status", "+ok")
            .add_child("data",
                json.create_node()
                    .add_text("name", "value")
                    .add_bool("success", true)
                    .add_number("number", 200));
        return res.write(json);
    });

    // start the server in alone mode
    server.run_alone("0.0.0.0|8194, 127.0.0.1|8195", "./httpd.cf");
    return 0;
}
```

### 3.13 Windows GUI sample

There is one Windows GUI sample with libfiber in [directory](samples/c/WinEchod). The screenshot is ![here](res/winecho.png)  

The server coroutine and client coroutine are all running in the same thread as the GUI, so you can operate the GUI object in server and client coroutine without worrying about the memory collision problem. And you can write network process with sequence way, other than asynchronus callback way which is so horrible. With the libfirber for Windows GUI, the asynchronous API like CAsyncSocket should be discarded. The network APIs are intergrated with the Windows GUI seamlessly because the libfiber using GUI message pump as event driven internal.

### 3.14 More SAMPLES

You can get more samples in [samples](samples/), which use many APIs in [acl project](https://github.com/acl-dev/acl/tree/master/lib_fiber/samples) library.
## 4 BUILDING
### 4.1 On Unix

```
$cd libfiber
$make
$cd samples
$make
```

<b>The simple Makefile shown below:</b>

```
fiber_server: fiber_server.c
	gcc -o fiber_server fiber_server.c patch.c -I{path_of_fiber_header} -L{path_of_fiber_lib) -lfiber -ldl -lpthread

fiber_client: fiber_client.c
	gcc -o fiber_client fiber_client.c patch.c -I{path_of_fiber_header} -L{path_of_fiber_lib) -lfiber -ldl -lpthread
```

### 4.2 On Windows

You can open the [fiber_vc2012.sln](c/fiber_vc2012.sln)/ [fiber_vc2013.sln](c/fiber_vc2013.sln)/[c/fiber_vc2015.sln](fiber_vc2015.sln) with vc2019, and build the libfiber library and the [samples](samples) included.

## 5 Benchmark

The picture below show the IOPS (io echo per-second) benchmark written by libfiber, comparing with the samples writen by [libmill](https://github.com/sustrik/libmill), golang and [libco](https://github.com/Tencent/libco). The samples written by libmill and libco are in [directory](benchmark), the sample written by golang is in [here](https://github.com/acl-dev/master-go/tree/master/examples/echo), and the sample written by libfiber is in [server sample directory](samples/c/server). The testing client is in [here](https://github.com/acl-dev/acl/tree/master/lib_fiber/samples/client2) from the [acl project](https://github.com/acl-dev/acl/).

![Benchmark](res/benchmark.png)

## 6 API support  

### 6.1 Base API

- acl_fiber_create  
- acl_fiber_self  
- acl_fiber_status  
- acl_fiber_kill   
- acl_fiber_killed  
- acl_fiber_signal  
- acl_fiber_yield  
- acl_fiber_ready  
- acl_fiber_switch  
- acl_fiber_schedule_init  
- acl_fiber_schedule  
- acl_fiber_schedule_with  
- acl_fiber_scheduled  
- acl_fiber_schedule_stop  
- acl_fiber_set_specific  
- acl_fiber_get_specific  
- acl_fiber_delay  
- acl_fiber_last_error  
- acl_fiber_last_serror  

### 6.2 IO API

- acl_fiber_recv  
- acl_fiber_recvfrom  
- acl_fiber_read  
- acl_fiber_readv  
- acl_fiber_recvmsg  
- acl_fiber_write  
- acl_fiber_writev  
- acl_fiber_send  
- acl_fiber_sendto  
- acl_fiber_sendmsg  
- acl_fiber_select  
- acl_fiber_poll  
- acl_fiber_close  

### 6.3 Net API

- acl_fiber_socket  
- acl_fiber_listen  
- acl_fiber_accept  
- acl_fiber_connect  
- acl_fiber_gethostbyname_r
- acl_fiber_getaddrinfo
- acl_fiber_freeaddrinfo

### 6.4 Channel API  

- acl_channel_create  
- acl_channel_free  
- acl_channel_send  
- acl_channel_send_nb  
- acl_channel_recv  
- acl_channel_recv_nb  
- acl_channel_sendp  
- acl_channel_recvp  
- acl_channel_sendp_nb  
- acl_channel_recvp_nb  
- acl_channel_sendul  
- acl_channel_recvul  
- acl_channel_sendul_nb  
- acl_channel_recvul_nb  

### 6.5 Sync API

<b>ACL_FIBER_MUTEX</b>  
- acl_fiber_mutex_create  
- acl_fiber_mutex_free  
- acl_fiber_mutex_lock  
- acl_fiber_mutex_trylock  
- acl_fiber_mutex_unlock  

<b>ACL_FIBER_RWLOCK</b>  
- acl_fiber_rwlock_create  
- acl_fiber_rwlock_free  
- acl_fiber_rwlock_rlock  
- acl_fiber_rwlock_tryrlock  
- acl_fiber_rwlock_wlock  
- acl_fiber_rwlock_trywlock  
- acl_fiber_rwlock_runlock  
- acl_fiber_rwlock_wunlock  

<b>ACL_FIBER_EVENT</b>  
- acl_fiber_event_create  
- acl_fiber_event_free  
- acl_fiber_event_wait  
- acl_fiber_event_trywait  
- acl_fiber_event_notify  

<b>ACL_FIBER_SEM</b>  
- acl_fiber_sem_create  
- acl_fiber_sem_free  
- acl_fiber_sem_wait  
- acl_fiber_sem_post  
- acl_fiber_sem_num  

## 7 About API Hook

On Linux/BSD/Mac, many IO and Net APIs are hooked. So you can just use the System standard APIs in your applications with libfiber, the hooked APIs will be replaced with libfiber APIs. In this case, you can <b>`coroutine`</b> your DB application with mysql driven and change nothing in mysql driven.  
The standard APIs been hooked are shown below:
- close
- sleep
- read
- readv
- recv
- recvfrom
- recvmsg
- write
- writev
- send
- sendto
- sendmsg
- sendfile64
- socket
- listen
- accept
- connect
- select
- poll
- epoll: epoll_create, epoll_ctl, epoll_wait
- gethostbyname(_r)
- getaddrinfo/freeaddrinfo

## 8 FAQ

1. <b>Is the coroutine schedule in multi-threads?</b>  
No. The coroutine schedule of libfiber is in one single thread. But you can start multiple threads that one thread has one schedule process.  
2. <b>How are the multi-cores of CPU used?</b>  
multiple threads can be started with its own coroutine schedule, each thread can ocpupy one CPU.  
3. <b>How does different threads mutex in coroutine schedule status?</b>  
Even though the OS system mutex APIs, such as pthread_mutex_t's APIs can be used, the ACL_FIBER_EVENT's APIs are recommended. It's safety when the OS system mutex APIs are used in short time without recursive invocation. But its unsafely using system mutex APIs in this case: One coroutine A1 of thread A had locked the thread-mutex-A, the coroutine A2 of thread A wanted to lock the thread-mutex-B which had been locked by one coroutine B1 of thread B, when the coroutine B2 of thread B wanted to lock the thread-mutex-A, thread deadlock happened! So, the coroutine mutex for threads and coroutines named ACL_FIBER_EVENT's APIs of libfiber were created, which can be used to make critical region between multiple coroutines in different threads(multiple continues in the same thread or not; it can also be used for different threads without coroutines).  
4. <b>Should the mysql-driven source codes be changed when used with libfiber?</b>  
In UNIX OS, the System IO APIs are hooked by libfiber, so nothing should be changed in mysql-driven.  
5. <b>How to avoid make the mysqld overloaded when many coroutines started?</b>  
The ACL_FIBER_SEM's APIs can be used to protect the mysqld being overloaded by many connections of many coroutines. These APIs can limit the connections number to the mysqld from coroutines.  
6. <b>Does the DNS domain resolving block the coroutine schedule?</b>  
No, because the System domain-resolving APIs such as gethostbyname(_r) and getaddrinfo are also hooked in libfiber.  
