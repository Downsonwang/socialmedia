#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <assert.h>
#include <iostream>


class SqlConnPool{
public:
   // 单例
   static SqlConnPool *Instance();

   // 初始化SQL连接池(创建sql连接)
   void Init(const char* host,int port,const char* username,const char* password,const char* database,int connSize);

   // 返回MYSQL对象
   MYSQL *GetConn();
   int GetFreeConnCount();

   // 用完sql连接之后,回收sql连接,重新入池
   void RecyConn(MYSQL * sql);
   void ClosePool();

private:
    SqlConnPool();
    ~SqlConnPool();
    // 复制构造函数和赋值操作符被显式禁用，不允许对该类进行对象的复制或赋值。
    SqlConnPool(const SqlConnPool&) = delete;
    SqlConnPool &operator=(const SqlConnPool&) = delete;

    int MAX_CONN_;
    int useCount_;
    int freeCount_;
    std::queue<MYSQL *> connQue_;
    std::mutex mtx_;
    // 协调多个线程访问资源
    sem_t semID_;
};


#endif