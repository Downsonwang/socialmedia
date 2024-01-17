#include "pool.h"


RedisConnPool::RedisConnPool(){

}


RedisConnPool::~RedisConnPool(){
    while(!redis_q.empty()){
        redisContext * tmp = redis_q.front();
        redisFree(tmp);
        tmp = NULL;
    }
}

// 单例模式
RedisConnPool* RedisConnPool::Instance(){
    static RedisConnPool * conn_redis = new RedisConnPool();
    return conn_redis;
}

// 初始化REDIS连接池
void RedisConnPool::Init(const char * ip,int port,int connSize,const struct timeval t){
    assert(connSize>0);
    MaxConn_ = connSize;

    if (t.tv_sec != 0 || t.tv_usec != 0){
        for (int i = 0; i < MaxConn_;i++){
            redisContext *r = redisConnectWithTimeout(ip,port,t);
            assert(r != NULL && !r->err);
            redis_q.push(std::move(r));
        }
    }else{
          for(int i = 0; i < MaxConn_; i++){
            redisContext *r = redisConnect(ip, port);
            assert(r != NULL && ! r->err);
            redis_q.push(std::move(r));
        }
    }

    //  0是线程 1是进程
    sem_init(&semID_,0,MaxConn_);
}

// 获取连接
redisContext * RedisConnPool::GetConn(){
    if(redis_q.empty()){
      //  LOG_INFO("Redis busy!");
      return nullptr;
    }
    // 控制信号量并发访问
    sem_wait(&semID_);

    std::unique_lock<std::mutex> lck(mtx_);
    // 获取一个连接
    redisContext * r = redis_q.front();
    redis_q.pop();
    return r;
}

// 回收连接 
void RedisConnPool::RedisRecycle(redisContext *r){
    assert(r);
   // 互斥锁保护
    std::unique_lock<std::mutex> lck(mtx_);
    redis_q.push(r);
    // 增加信号量 表示一个连接可用
    sem_post(&semID_);
}