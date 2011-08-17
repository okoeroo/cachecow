#ifndef CALLBACKS_H
#define CALLBACKS_H

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



void conn_readcb(struct bufferevent *, void *);
void conn_writecb(struct bufferevent *, void *);
void conn_eventcb(struct bufferevent *, short, void *);
void signal_cb(evutil_socket_t, short, void *);
void accept_conn_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int, void *);
void accept_error_cb(struct evconnlistener *, void *);

#endif /* CALLBACKS_H */
