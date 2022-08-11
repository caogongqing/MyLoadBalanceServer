#include "buf_pool.h"
#include <assert.h>

//单例的对象
buf_pool* buf_pool::_instance = NULL;

//用于保证创建单例的一个方法全局只执行一次
pthread_once_t buf_pool::_once = PTHREAD_ONCE_INIT;

//初始化锁
pthread_mutex_t buf_pool::_mutex = PTHREAD_MUTEX_INITIALIZER;


void buf_pool::make_io_buf_list(int cap, int num)
{
    //链表的头指针
    io_buf *prev;

    //开辟4k buf内存池 
    _pool[cap] = new io_buf(cap);
    if (_pool[cap] == NULL) {
        fprintf(stderr, "new io_buf %d error", cap);
        exit(1);
    }

    prev = _pool[cap];

    for (int i = 1; i < num; i++) {
        prev->next = new io_buf(cap);
        if (prev->next == NULL) {
            fprintf(stderr, "new io_buf cap error");
            exit(1);
        }
        prev = prev->next;
    }

    _total_mem += cap/1024 * num;    
}




//构造函数私有化
//buf_pool --> [m4K] ---> (io_buf)..io_buf..io_buf
//             [m16K] --> io_buf ..io_buf ..io_buf
//             ...
buf_pool::buf_pool():_total_mem(0)
{
    make_io_buf_list(m4K, 5000);
    make_io_buf_list(m16K, 1000);
    make_io_buf_list(m64K, 500);
    make_io_buf_list(m256K,200);
    make_io_buf_list(m1M, 50);
    make_io_buf_list(m4M, 20);
    make_io_buf_list(m8M, 10);
}


//从内存池申请一块内存
io_buf *buf_pool::alloc_buf(int N)
{
    int index;
    // 1 找到N最接近的 一个刻度链表 返回一个io_buf
    if (N <= m4K) {
        index = m4K;
    }
    else if ( N <= m16K) {
        index = m16K;
    }
    else if ( N <= m64K) {
        index = m64K;
    }
    else if ( N <= m256K) {
        index = m256K;
    }
    else if ( N <= m1M) {
        index = m1M;
    }
    else if ( N <= m4M) {
        index = m4M;
    }
    else if ( N <= m8M) {
        index = m8M;
    }
    else {
        return NULL;
    }


    //2 如果该index内存已经没有了，需要额外的申请内存 
    pthread_mutex_lock(&_mutex);
    if (_pool[index] == NULL) {
        //index链表为空，需要新申请index大小的io_buf
        
        if ( _total_mem + index/1024 >= MEM_LIMIT ) {
            fprintf(stderr, "already use too many memory!\n");
            exit(1);
        }

        io_buf *new_buf = new io_buf(index);
        if (new_buf == NULL) {
            fprintf(stderr, "new io_buf error\n");
            exit(1);
        }

        _total_mem += index/1024;
        pthread_mutex_unlock(&_mutex);
        return new_buf;
    }

    //3 如果index有内存， 从pool中拆除一块内存返回
    io_buf *target = _pool[index];
    _pool[index] = target->next;
    pthread_mutex_unlock(&_mutex);

    target->next = NULL;

    return target;
}

io_buf *buf_pool::alloc_buf()
{
    return alloc_buf(m4K);
}

//重置一个io_buf 放回pool中
void buf_pool::revert(io_buf *buffer)
{
    //将buffer放回pool中
    //index 是属于pool中哪个链表的
    int index = buffer->capacity; 

    buffer->length = 0;
    buffer->head = 0;

    pthread_mutex_lock(&_mutex);

    //断言，一定能够找到index key
    assert(_pool.find(index) != _pool.end());
#if 0
    if (_pool.find(index) == _pool.end()) {
        exit(1);
    }
#endif

    //将buffer设置为对应 buf链表的首节点
    buffer->next = _pool[index];
    _pool[index] = buffer;

    pthread_mutex_unlock(&_mutex);
}
