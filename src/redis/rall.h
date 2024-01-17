#ifndef REDISCONNRAII
#define REDISCONNRAII

#include "pool.h"
#include <hiredis/hiredis.h>

class RedisConnRAII{

public:
    RedisConnRAII(redisContext ** r,RedisConnPool * conn){
        // 将传入的 **r 给初始化
        *r = conn->GetConn();

        // 将*r和*conn 放入RAII 
        r_ = *r;
        redisconn_ = conn;
    }
    ~RedisConnRAII(){
        if (r_) {
            redisconn_->RedisRecycle(r_);
        }
    }
private:
    redisContext * r_;
    RedisConnPool * redisconn_;
};


#endif