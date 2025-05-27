// file: simple_libevent_server.c

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>

#define LISTEN_PORT 9999

void read_cb(struct bufferevent *bev, void *ctx) {
    char buffer[1024];
    size_t n;
    while ((n = bufferevent_read(bev, buffer, sizeof(buffer))) > 0) {
        bufferevent_write(bev, buffer, n);
    }
}

void event_cb(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_EOF) {
        printf("Connection closed.\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on the connection: %s\n", strerror(errno));
    }
    bufferevent_free(bev);
}

void accept_cb(struct evconnlistener *listener, evutil_socket_t fd,
               struct sockaddr *addr, int socklen, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);

    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    const char *msg = "Hello from libevent server!\n";
    bufferevent_write(bev, msg, strlen(msg));
}

int main() {
    struct event_base *base;
    struct evconnlistener *listener;
    struct sockaddr_in sin;

    base = event_base_new();
    if (!base) {
        perror("event_base_new() failed");
        return 1;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0); // 0.0.0.0
    sin.sin_port = htons(LISTEN_PORT);

    listener = evconnlistener_new_bind(base, accept_cb, NULL,
                                       LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                       (struct sockaddr*)&sin, sizeof(sin));
    if (!listener) {
        perror("evconnlistener_new_bind() failed");
        return 1;
    }

    printf("Listening on port %d...\n", LISTEN_PORT);
    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_base_free(base);
    return 0;
}
