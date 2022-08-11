#include "thread_pool.h"


//提供一个获取一个thread_queue方法
thread_queue<task_msg> * thread_pool::get_thread()
{
    if (_index == _thread_cnt)     {
        _index = 0;
    }
    return _queues[_index++];
}


//IO事件触发的回调函数
//typedef void io_callback(event_loop *loop, int fd, void *args);

/*
 * 一旦有task任务消息过来， 这个业务函数就会被loop监听到并执行,读出消息队列里的消息并进行处理
 * */
void deal_task(event_loop *loop, int fd, void *args)
{
    //1 从thread_queue中去取数据
    thread_queue<task_msg>* queue = (thread_queue<task_msg>*)args;

    //取出queue 调用recv()方法
    std::queue<task_msg> new_task_queue;
    queue->recv(new_task_queue);//new_task_queue就是存放的 thread_queue的全部任务

    //依次处理每个任务
    while (new_task_queue.empty() != true) {
        //从队列中得到一个任务
        task_msg task = new_task_queue.front();

        //将这个任务和队列中弹出
        new_task_queue.pop();

        if (task.type == task_msg::NEW_CONN) {
            //如果是任务1: 中取出来的数据 就应该是一个刚刚accept成功的connfd
            tcp_conn *conn = new tcp_conn(task.connfd, loop);
            //让当前线程去创建一个链接，同时将这个链接的connfd加入到当前thrad的event_loop中进行监听
            if (conn == NULL) {
                fprintf(stderr, "in thread new tcp_conn error\n");
                exit(1);
            }

            printf("[thread]: create net connection succ !\n");
        }
        else if (task.type == task_msg::NEW_TASK) {
            //如果是任务2: 是一个普通任务
            //将该任务添加到event_loop 循环中，去执行
            loop->add_task(task.task_cb, task.args);
        }
        else {
            fprintf(stderr, "unknown task!\n");
        }
    }
}


//线程的主业务函数
void *thread_main(void *args)
{
    thread_queue<task_msg> *queue = (thread_queue<task_msg>*)args;

    event_loop *loop = new event_loop();
    if (loop == NULL) {
        fprintf(stderr, "new event_loop error");
        exit(1);
    }

    queue->set_loop(loop);
    queue->set_callback(deal_task, queue);

    //启动loop监听
    loop->event_process();

    return NULL;
}


//初始化线程池的构造函数 每次创建一个server 就应该创建一个线程池
thread_pool::thread_pool(int thread_cnt)
{
    _index = 0;
    _queues = NULL;
    _thread_cnt = thread_cnt;
    if (_thread_cnt <= 0) {
        fprintf(stderr, "thread_cnt need > 0\n");
        exit(1);
    }

    //创建thread_queue
    _queues = new thread_queue<task_msg>*[thread_cnt];// 开辟了 cnt个 thread_queue指针，每个指针并没有真正new对象
    _tids = new pthread_t[thread_cnt];


    //开辟线程
    int ret;
    for (int i = 0; i < thread_cnt; i++) {
        //给一个thread_queue开辟内存，创建对象
        _queues[i] = new thread_queue<task_msg>();

        //第i个线程 绑定第i个thraed_queue
        ret = pthread_create(&_tids[i], NULL, thread_main, _queues[i]);
        if (ret == -1){
            perror("thread_pool create error");
            exit(1);
        }

        printf("create %d thread\n", i);

        //将线程设置detach模式 线程分离模式
        pthread_detach(_tids[i]);
    }

    printf("====> new thread_pool <==== \n");
    
    return ;
}


//发送一个task NEW_TASK类型的任务接口
void thread_pool::send_task(task_func func, void *args)
{
    //给当前thread_pool中每一个thread里的thread_queue去发送当前task
    task_msg task;

    //制作一个任务
    task.type = task_msg::NEW_TASK;
    task.task_cb = func;
    task.args = args;

    for (int i = 0; i < _thread_cnt; i++) {
        //取出第i个线程的消息队列queue
        thread_queue<task_msg> *queue = _queues[i];

        //给queue发送一个task任务
        queue->send(task);
    }
}

