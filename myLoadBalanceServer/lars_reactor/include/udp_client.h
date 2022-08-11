#pragma once

#include <netinet/in.h>
#include <signal.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "net_connection.h"
#include "event_loop.h"
#include "message.h"

class udp_client:public net_connection
{
public:

    udp_client(event_loop *loop, const char *ip, uint16_t port);

    ~udp_client();

    //udp 主动发送消息的方法
    virtual int send_message(const char *data, int msglen, int msgid);
    virtual int get_fd() {
        return this->_sockfd;
    }

    void do_read();

    void add_msg_router(int msgid, msg_callback *cb, void *user_data = NULL);

private:
    int _sockfd;

    char _read_buf[MESSAGE_LENGTH_LIMIT];
    char _write_buf[MESSAGE_LENGTH_LIMIT];

    event_loop *_loop;

    //消息路由分发机制
    msg_router _router;
};
