/*
 * redis_list_test.cpp
 *
 *  Created on: 2017年10月22日
 *      Author: ym
 */
#include <iostream>
#include <list>
#include "redis_list_test.hpp"

using namespace std;

static list<redisContext * > redis;

void redis_list_test()
{
	redisContext *ctx;

	if (redis.size() == 0)
		ctx = redisConnect("127.0.0.1", 6379);
	else{
		ctx = redis.front();
		redis.pop_front();
	}
	redisReply *reply = (redisReply *)redisCommand(ctx, "set test 123");
	freeReplyObject(reply);

	if (redis.size() > 100)
		redisFree(ctx);
	else
		redis.push_back(ctx);
}



