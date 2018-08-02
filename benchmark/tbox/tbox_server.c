#include "tbox/tbox.h"
#if defined(_WIN32) || defined(_WIN64)
#   include "../patch.h"
#else
#   include <getopt.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <string.h>

static tb_void_t tb_demo_coroutine_client(tb_cpointer_t priv)
{
    // check
    tb_socket_ref_t sock = (tb_socket_ref_t)priv;
    tb_assert_and_check_return(sock);

    // loop
    tb_byte_t data[8192] = {0};
    tb_size_t size = 13;
    while (1)
    {
        // recv data
        if (tb_socket_brecv(sock, data, size))
        {
            // send data
            if (!tb_socket_bsend(sock, data, size))
            {
                // error
                tb_trace_e("send error!");
                break;
            }
        }
        else break;
    }

    // trace
    tb_trace_d("echo: %s", data);

    // exit socket
    tb_socket_exit(sock);
}

static int  __local_port    = 9101;

static tb_void_t tb_demo_coroutine_listen(tb_cpointer_t priv)
{
    // done
    tb_socket_ref_t sock = tb_null;
    do
    {
        // init socket
        sock = tb_socket_init(TB_SOCKET_TYPE_TCP, TB_IPADDR_FAMILY_IPV4);
        tb_assert_and_check_break(sock);

        // bind socket
        tb_ipaddr_t addr;
        tb_ipaddr_set(&addr, tb_null, __local_port, TB_IPADDR_FAMILY_IPV4);
        if (!tb_socket_bind(sock, &addr))
        {
            printf("bind error=%s, port=%d\r\n", strerror(errno), __local_port); 
            break;
	}

        // listen socket
        if (!tb_socket_listen(sock, 128))
        {
            printf("listen error=%s, port=%d\r\n", strerror(errno), __local_port); 
            break;
        }

        printf("listen %d ok\r\n", __local_port);

        // trace

        // accept client sockets
        tb_socket_ref_t client = tb_null;
        while (1)
        {
            // accept and start client connection
            if ((client = tb_socket_accept(sock, tb_null)))
            {
                printf("accept one fd=%d\r\n", client); 
                if (!tb_coroutine_start(tb_null, tb_demo_coroutine_client, client, 0)) break;
            }
            else if (tb_socket_wait(sock, TB_SOCKET_EVENT_ACPT, -1) <= 0) break;
        }

    } while (0);

    // exit socket
    if (sock) tb_socket_exit(sock);
    sock = tb_null;
}

static void usage(const char *procname)
{
    tb_printf("usage: %s -h[help] -s ip -p port\r\n", procname);
}

int main(int argc, char *argv[])
{
    // init tbox
    if (!tb_init(tb_null, tb_null)) return -1;

    // parse arguments
    char ip[128];
    int port = 9101, ch;
    tb_snprintf(ip, sizeof(ip), "127.0.0.1");
    while ((ch = getopt(argc, argv, "hs:p:")) > 0) 
    {
        switch (ch) 
        {
        case 'h':
            usage(argv[0]);
            return 0;
        case 's':
            tb_snprintf(ip, sizeof(ip), "%s", optarg);
            break;
        case 'p':
            port = tb_atoi(optarg);
            break;
        default:
            break;
        }
    }

    // init listen address
    tb_ipaddr_t addr;
    tb_ipaddr_set(&addr, ip, port, TB_IPADDR_FAMILY_IPV4);

    // init scheduler
    tb_co_scheduler_ref_t scheduler = tb_co_scheduler_init();
    if (scheduler)
    {
        // start listener
        tb_coroutine_start(scheduler, tb_demo_coroutine_listen, tb_null, 0);

        tb_printf("begin run ...\r\n");

        // run scheduler
        tb_co_scheduler_loop(scheduler, tb_true);

        tb_printf("begin exit ...\r\n");
        // exit scheduler
        tb_co_scheduler_exit(scheduler);
    }

    // exit tbox
    tb_exit();
    return 0;
}
