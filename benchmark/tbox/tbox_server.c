#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include "tbox.h"

// port
#define TB_DEMO_PORT        (9090)

// timeout
#define TB_DEMO_TIMEOUT     (-1)

static tb_void_t tb_demo_coroutine_client(tb_cpointer_t priv)
{
    // check
    tb_socket_ref_t sock = (tb_socket_ref_t)priv;
    tb_assert_and_check_return(sock);

    // read data
    tb_char_t data[8192] = {0};
    tb_size_t read = 0;
    tb_size_t size = sizeof(data) - 1;
    tb_long_t wait = 0;
    while (read < size)
    {
        // read it
        tb_long_t real = tb_socket_recv(sock, (tb_byte_t*)data + read, size - read);

        // has data?
        if (real > 0) 
        {
            read += real;
            wait = 0;
        }
        // no data? wait it
        else if (!real && !wait)
        {
            tb_trace_i("wait ..");
            // wait it
            wait = tb_socket_wait(sock, TB_SOCKET_EVENT_RECV, TB_DEMO_TIMEOUT);
            tb_assert_and_check_break(wait >= 0);
        }
        // failed or end?
        else break;
    }

    // trace
    tb_trace_i("echo: %s", data);

    // exit socket
    tb_socket_exit(sock);
}

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
        tb_ipaddr_set(&addr, tb_null, TB_DEMO_PORT, TB_IPADDR_FAMILY_IPV4);
        if (!tb_socket_bind(sock, &addr)) break;

        // listen socket
        if (!tb_socket_listen(sock, 1000)) break;

        // trace
        tb_trace_i("listening ..");

        // wait accept events
        while (tb_socket_wait(sock, TB_SOCKET_EVENT_ACPT, -1) > 0)
        {
            // accept client sockets
            tb_socket_ref_t client = tb_null;
            while ((client = tb_socket_accept(sock, tb_null)))
            {
                // start client connection
                if (!tb_coroutine_start(tb_null, tb_demo_coroutine_client, client, 0)) break;
            }
        }
    } while (0);

    // exit socket
    if (sock) tb_socket_exit(sock);
    sock = tb_null;
}

static void usage(const char *procname)
{
	printf("usage: %s -h[help] -s ip -p port\r\n", procname);
}

int main(int argc, char *argv[])
{
	char ip[128];
	int port = 9001, ch;

	snprintf(ip, sizeof(ip), "127.0.0.1");

	while ((ch = getopt(argc, argv, "hs:p:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			snprintf(ip, sizeof(ip), "%s", optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		default:
			break;
		}
	}

	tb_co_scheduler_ref_t scheduler = tb_co_scheduler_init();
	tb_coroutine_start(scheduler, tb_demo_coroutine_listen, tb_null, 0);
	tb_co_scheduler_loop(scheduler, tb_true);
	tb_co_scheduler_exit(scheduler);

	return 0;
}
