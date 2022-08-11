#pragma  once

#include "thread_queue.h"
#include "task_msg.h"
#include "tcp_conn.h"

class thread_pool
{
public:

    //初始化线程池的构造函数
    thread_pool(int thread_cnt);

    //提供一个获取一个thread_queue方法
    thread_queue<task_msg> * get_thread();


    //发送一个task NEW_TASK类型的任务接口
    void send_task(task_func func, void *args = NULL);


private:
    //当前的thread_queue集合
    thread_queue<task_msg>** _queues;

    //线程个数
    int _thread_cnt;

    //线程ID
    pthread_t*  _tids;

    //获取线程的index索引
    int _index;
};
