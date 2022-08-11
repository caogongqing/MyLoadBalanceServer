#include "store_report.h"
#include <string>

using namespace std;

StoreReport::StoreReport()
{
    //初始化    
    
    mysql_init(&_db_conn);
    mysql_options(&_db_conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
    my_bool reconnect = 1;
    mysql_options(&_db_conn, MYSQL_OPT_RECONNECT, &reconnect);

    //2 加载配置文件
    string db_host = config_file::instance()->GetString("mysql", "db_host", "127.0.0.1");
    short db_port = config_file::instance()->GetNumber("mysql", "db_port", 3306);
    string db_user = config_file::instance()->GetString("mysql", "db_user", "root");
    string db_passwd = config_file::instance()->GetString("mysql", "db_passwd", "aceld");
    string db_name = config_file::instance()->GetString("mysql", "db_name", "lars_dns");
    
    //3 连接数据库
    if (mysql_real_connect(&_db_conn, db_host.c_str(), db_user.c_str(), db_passwd.c_str(), db_name.c_str(), db_port, NULL, 0) == NULL) {
        fprintf(stderr, "Failed to connect mysql: %s\n", mysql_error(&_db_conn));
        exit(1);
    }
}

void StoreReport::store(lars::ReportStatusRequest &req)
{
    for (int i = 0; i < req.results_size(); i++) {
        const lars::HostCallResult &result = req.results(i);
        int overload = result.overload() ? 1:0;
        
        char sql[1024];
        memset(sql, 0, 1024);
        
        snprintf(sql, 1024, "INSERT INTO ServerCallStatus"
                "(modid, cmdid, ip, port, caller, succ_cnt, err_cnt, ts, overload) "
                "VALUES (%d, %d, %u, %u, %u, %u, %u, %u, %d) ON DUPLICATE KEY "
                "UPDATE succ_cnt = %u, err_cnt = %u, ts = %u, overload = %d", 
                req.modid(), req.cmdid(), result.ip(), result.port(), req.caller(),
                result.succ(), result.err(), req.ts(), overload,
                result.succ(), result.err(), req.ts(), overload);
   //     printf("sql = %s\n", sql);


        if (mysql_real_query(&_db_conn, sql, strlen(sql)) != 0) {
            fprintf(stderr, "insert to ServerCallStatus error %s\n", mysql_error(&_db_conn));
        }
    }
}
