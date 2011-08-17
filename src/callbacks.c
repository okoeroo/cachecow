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


#include "callbacks.h"

const char MESSAGE[] = "foo1\n";


void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
    printf("%s - begin\n", __func__);
    struct event_base *base = user_data;
    struct timeval delay = { 1, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

    event_base_loopexit(base, &delay);
    printf("%s - end\n", __func__);
}



void conn_eventcb(struct bufferevent *bev, short events, void *user_data)
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


void conn_readcb(struct bufferevent *bev, void *user_data)
{
    printf("%s - begin\n", __func__);
    struct evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) {
        printf("flushed answer\n");
        bufferevent_free(bev);
    }
    printf("%s - end\n", __func__);
}

void conn_writecb(struct bufferevent *bev, void *user_data)
{
    printf("%s - begin\n", __func__);
    struct evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) {
        printf("flushed answer\n");
        bufferevent_free(bev);
    }
    printf("%s - end\n", __func__);
}


void 
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

    /* bufferevent_enable(bev, EV_WRITE); */
    /* bufferevent_disable(bev, EV_READ); */

    bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
    printf("%s - end\n", __func__);
}

void
accept_error_cb(struct evconnlistener *lis, void *ptr) {
    printf ("%s: Error: Something is wrong");
}

