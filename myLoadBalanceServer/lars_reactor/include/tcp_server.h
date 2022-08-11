#pragma once

#include <netinet/in.h>
#include "event_loop.h"
#include "tcp_conn.h"
#include "message.h"
#include "thread_pool.h"

class tcp_server
{
public:
    //构造函数
    tcp_server(event_loop * loop, const char *ip, uint16_t port);

    //开始提供创建链接的服务
    void do_accept();

    //析构函数  资源的释放
    ~tcp_server();

private:
    int _sockfd; //套接字 listen fd
    struct sockaddr_in  _connaddr; //客户端链接地址
    socklen_t _addrlen; //客户端链接地址长度


    //event_loop epoll的多路IO复用机制
    event_loop *_loop;

public:
    static tcp_conn*  *conns; //全部已经在线的连接 
    static void increase_conn(int connfd, tcp_conn *conn); //新增一个链接
    static void decrease_conn(int connfd); //减少一个链接
    static void get_conn_num(int *curr_conn); //得到当前的连接刻度

    static pthread_mutex_t _conns_mutex;//保护conns添加删除的互斥锁
#define MAX_CONNS 3 //从配置文件中读的 TODO
    static int _max_conns; //当前允许连接的最大个数
    static int _curr_conns; //当前所管理的连接个数

    //添加一个路由分发机制句柄
    static msg_router router;

    //提供一个添加路由的方法 给 开发者提供的API
    void add_msg_router(int msgid, msg_callback *cb, void *user_data = NULL) {
        router.register_msg_router(msgid, cb, user_data);
    }


    //设置链接创建之后Hook函数  API
    static void set_conn_start(conn_callback cb, void *args = NULL)
    {
        conn_start_cb = cb;
        conn_start_cb_args = args;
    }

    //设置链接销毁之前Hook函数 API
    static void set_conn_close(conn_callback cb, void *args = NULL)
    {
        conn_close_cb = cb;
        conn_close_cb_args = args;
    }

    //创建链接之后 要触发的回调函数
    static conn_callback conn_start_cb;      
    static void * conn_start_cb_args;

    //销毁链接之前 要触发的回调函数
    static conn_callback conn_close_cb;
    static void * conn_close_cb_args;

    thread_pool * get_th_pool() {
        return _thread_pool;
    }

private:
    //==========链接池=========
    thread_pool *_thread_pool;
};


