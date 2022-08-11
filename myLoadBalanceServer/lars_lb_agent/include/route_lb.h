#pragma once
#include "load_balance.h"


//key-->modid/cmdid  value--> load_balance
typedef __gnu_cxx::hash_map<uint64_t, load_balance*> route_map;
typedef __gnu_cxx::hash_map<uint64_t, load_balance*>::iterator route_map_it;


/*
 * 针对 多组 modid/cmdid 和load_balance的对应关系
 *
 * 目前 route_lb对象只有3个，每个udp server 都会有一个route_lb对象
 *
 * */
class route_lb
{
public:
    route_lb(int id);

    //agent 获取一个host主机 将返回的主机结果存放在rsp中
    int get_host(int modid, int cmdid, lars::GetHostResponse &rsp);

    //agent 获取host主机集合 将返回的主机结果存放在rsp中
    int get_route(int modid, int cmdid, lars::GetRouteResponse &rsp);

    int update_host(int modid, int cmdid, lars::GetRouteResponse &rsp);

    //agent上报某个主机的结果
    void report_host(lars::ReportRequest &req);

    //将全部的load_balance都重置为NEW状态
    void reset_lb_status();

private:
    route_map _route_lb_map;  //当前route_lb模块所管理的loadbalance集合
    pthread_mutex_t _mutex;
    int _id;                  //当前route_lb 的编号 和 udp server是一一对应的
};
