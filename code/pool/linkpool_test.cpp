// #include <mysql/mysql.h>
// #include <stdio.h>
// #include <string>
// #include <queue>
// #include <mutex>
// #include <semaphore.h>
// #include <thread>
// #include "sqlconnpool.h"
// #include "sqlconnRAII.h"
// #include "../log/log.h"

// int main()
// {
    
//     MYSQL* sql;
//     SqlConnPool*pool = SqlConnPool::Instance();
//     pool->Init("local", 3306, "root", "Tly13956203250.", "webserver", 10);
//     //*sql = pool->GetConn();
//     SqlConnRAII(&sql,  SqlConnPool::Instance());
//     assert(sql);

//     return 0;
// }
