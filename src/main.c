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

typedef enum protocol_e {
    IPv6TCP,
    IPv6UDP,
    IPv4TCP,
    IPv4UDP,
} protocol_t;

static const char MESSAGE[] = "foo1\n";

static void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int socklen, void *);
static void conn_writecb(struct bufferevent *, void *);
static void conn_eventcb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);



static void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
    printf("%s - begin\n", __func__);
    struct event_base *base = user_data;
    struct timeval delay = { 1, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

    event_base_loopexit(base, &delay);
    printf("%s - end\n", __func__);
}



static void conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
    printf("%s - begin\n", __func__);
    if (events & BEV_EVENT_EOF) {
        printf("Connection closed.\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on the connection: %s\n", strerror(errno));/*XXX win32*/
    }
    /* None of the other events can happen here, since we haven't enabled
     * timeouts */
    bufferevent_free(bev);
    printf("%s - end\n", __func__);
}


static void conn_readcb(struct bufferevent *bev, void *user_data)
{
    printf("%s - begin\n", __func__);
    struct evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) {
        printf("flushed answer\n");
        bufferevent_free(bev);
    }
    printf("%s - end\n", __func__);
}

static void conn_writecb(struct bufferevent *bev, void *user_data)
{
    printf("%s - begin\n", __func__);
    struct evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) {
        printf("flushed answer\n");
        bufferevent_free(bev);
    }
    printf("%s - end\n", __func__);
}


static void 
accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data) {
    struct event_base *base = user_data;
    struct bufferevent *bev;

    printf("%s - begin\n", __func__);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(base);
        return;
    }
    bufferevent_setcb(bev, conn_readcb, conn_writecb, conn_eventcb, NULL);
    bufferevent_enable(bev, EV_WRITE);
    bufferevent_disable(bev, EV_READ);

    bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
    printf("%s - end\n", __func__);
}

static void
accept_error_cb(struct evconnlistener *lis, void *ptr) {
    printf ("%s: Error: Something is wrong");
}


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
    return doit(6666, IPv6TCP, 8000);
}
