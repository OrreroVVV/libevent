#include <event2/event.h>
#include <event2/bufferevent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9999

void write_cb(struct bufferevent *bev, void *ctx) {
    const char *msg = "Hello from libevent client!\n";
    bufferevent_write(bev, msg, strlen(msg));
    // 发送完消息后不再关注写事件，避免重复发送
    bufferevent_disable(bev, EV_WRITE);
}

void read_cb(struct bufferevent *bev, void *ctx) {
    char buffer[1024];
    int n;
    while ((n = bufferevent_read(bev, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, n, stdout);
    }
}

// 事件回调（错误、关闭等）
void event_cb(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_CONNECTED) {
        printf("Connected to server.\n");
    } else if (events & BEV_EVENT_EOF) {
        printf("Connection closed by server.\n");
        bufferevent_free(bev);
        struct event_base *base = (event_base*)ctx;
        event_base_loopexit(base, NULL);  // 退出事件循环
    } else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on connection: %s\n", strerror(errno));
        bufferevent_free(bev);
        struct event_base *base = (event_base*)ctx;
        event_base_loopexit(base, NULL);  // 退出事件循环
    }
}

int main() {
    struct event_base *base;
    struct bufferevent *bev;
    struct sockaddr_in server_addr;

    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return 1;
    }

    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Could not create bufferevent!\n");
        event_base_free(base);
        return 1;
    }

    bufferevent_setcb(bev, read_cb, write_cb, event_cb, base);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address\n");
        bufferevent_free(bev);
        event_base_free(base);
        return 1;
    }

    if (bufferevent_socket_connect(bev, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error starting connection\n");
        bufferevent_free(bev);
        event_base_free(base);
        return 1;
    }

    event_base_dispatch(base);

    event_base_free(base);
    return 0;
}
