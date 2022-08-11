#include "lars_reactor.h"
#include "lars.pb.h"
#include "mysql.h"

class StoreReport
{
public:
    StoreReport();
    
    void store(lars::ReportStatusRequest &req);
private:
    MYSQL _db_conn;
};

void *store_main(void*);
