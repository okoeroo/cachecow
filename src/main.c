#include "main.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifndef WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include "cc_types.h"
#include "callbacks.h"


int
doit (int listen_port, protocol_t proto, int conn_port) {
    struct event_base *base;
    struct evconnlistener *listener;
    struct event *signal_event;

    struct sockaddr_in6 sin;
    /* struct sockaddr_storage sin; */

    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return 1;
    }

    memset(&sin, 0, sizeof(sin));

    sin.sin6_family = AF_INET6;
    sin.sin6_port = htons(listen_port);

    listener = evconnlistener_new_bind(base,
                                       accept_conn_cb,
                                       (void *)base,
                                       LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,
                                       -1,
                                       (struct sockaddr*)&sin,
                                       sizeof(sin));
    if (!listener) {
        fprintf(stderr, "Could not bind to port %d\n", listen_port);
        return 1;
    }
    evconnlistener_set_error_cb(listener, accept_error_cb);

    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
    if (!signal_event || event_add(signal_event, NULL)<0) {
        fprintf(stderr, "Could not create/add a signal event!\n");
        return 1;
    }

    /* Start */
    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_free(signal_event);
    event_base_free(base);

}

int
main(int argc, char **argv)
{
    info_t info;


    return doit(6666, IPv6TCP, 8000);
}
