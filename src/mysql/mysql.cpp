/*
 * @Descripttion:  MYSQL_POOL_CONN
 * @Author: DW
 * @Date: 2024-01-15 10:45:13
 * @LastEditTime: 2024-01-15 11:09:51
 */
#include "mysql/pool.h"

// 初始化构造函数
SqlConnPool::SqlConnPool(){
    useCount_ = 0;
    freeCount_ = 0;
}

SqlConnPool* SqlConnPool::Instance(){
    static SqlConnPool * connpool = new SqlConnPool();
    return connpool;
}

void SqlConnPool::Init(const char* host,int port,const char* username,const char* password,const char* database,int connSize = 10){
    assert(connSize>0);
    for (int i = 0; i < connSize;i++){
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if(!sql){
            assert(sql);
        }

        sql = mysql_real_connect(sql,host,username,password,database,port,nullptr,0);
        if (!sql){
            assert(sql);
        }
        connQue_.push(sql);
    }
    MAX_CONN_ = connSize;

    // 初始化信号量计数器（信号量semID_最多有MAX_CONN_来执行任务， 
        // 其中第二个参数0表示线程 非0表示进程）
        sem_init(&semID_, 0, MAX_CONN_);

}

MYSQL * SqlConnPool::GetConn(){
    MYSQL * sql = nullptr;
    if(connQue_.empty()){
        return nullptr;
    }
}


void SqlConnPool::RecyConn(MYSQL * sql){
    assert(sql);

    std::unique_lock<std::mutex> my_unique_lock(mtx_);
    connQue_.push(sql);

    // 增加信号计数器的数字
    sem_post(&semID_);
}

void SqlConnPool::ClosePool(){
    std::unique_lock<std::mutex> my_unique_lock(mtx_);
    while(!connQue_.empty()){
        MYSQL * sql = connQue_.front();
        connQue_.pop();
        mysql_close(sql);
    }
    mysql_library_end();
}


int SqlConnPool::GetFreeConnCount(){
  std::unique_lock<std::mutex> my_unique_lock(mtx_);
  return connQue_.size();

}


SqlConnPool::~SqlConnPool(){
    ClosePool();
}