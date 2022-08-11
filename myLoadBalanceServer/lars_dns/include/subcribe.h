#pragma once

#include <pthread.h>
#include <ext/hash_map>
#include <ext/hash_set>
#include "dns_route.h"
#include "lars_reactor.h"

using namespace __gnu_cxx;

//定义一个订阅列表的关系类型 key-> modid/cmdid,  value->fds(集合)
typedef hash_map<uint64_t, hash_set<int>> subcribe_map;

//定义一个发布列表的关系类型  key-> fd(订阅客户端的连接), value -> modid/cmdid
typedef hash_map<int, hash_set<uint64_t>> publish_map;



class SubcribeList
{
public:
    static void init() {
        _instance = new SubcribeList();
    }

    //单例模式
    static SubcribeList *instance() {
        pthread_once(&_once, init);
        return _instance;
    } 


    //订阅功能
    void subcribe(uint64_t mod, int fd);
    
    //取消订阅功能
    void unsubcribe(uint64_t mod, int fd);
    
    //发布功能
    //输入形参： 是那些modid被修改了，被修改的模块所对应的fd就应该被发布，收到新的modid/cmdid的结果
    void publish(std::vector<uint64_t> &change_mods);

    void make_publish_map(listen_fd_set &online_fds, publish_map &need_publish);

    publish_map * get_push_list() {
        return &_push_list;
    }
    
private:
    SubcribeList();
    SubcribeList(const SubcribeList&);
    const SubcribeList& operator=(const SubcribeList&);
    static SubcribeList *_instance;
    static pthread_once_t _once;


    //订阅清单
    subcribe_map _book_list;
    pthread_mutex_t _book_list_lock;
    
    //发布清单
    publish_map _push_list;
    pthread_mutex_t _push_list_lock;
};
