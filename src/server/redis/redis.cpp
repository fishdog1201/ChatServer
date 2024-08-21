#include <iostream>
#include "redis.hpp"

Redis::Redis()
    :_publish_context(nullptr), _subscribe_context(nullptr) {}

Redis::~Redis()
{
    if (!_publish_context) {
        redisFree(_publish_context);
    }
    if (!_subscribe_context) {
        redisFree(_subscribe_context);
    }
}

bool Redis::connect()
{
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (_publish_context == nullptr) {
        std::cerr << "publish connect redis failed.\n";
        return false;
    }

    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if (_subscribe_context == nullptr) {
        std::cerr << "subscribe connect redis failed.\n";
        return false;
    }

    std::thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    std::cout << "Connect redis success.\n";
    return true;
}

bool Redis::publish(int channel, std::string message)
{
    redisReply* reply = (redisReply*)redisCommand(_publish_context, "PUBLISH %d %s", channel, message);
    if (reply == nullptr) {
        std::cerr << "publish message failed.\n";
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    if (redisAppendCommand(this->_subscribe_context, "SUBSCRIBE %d", channel) == REDIS_ERR) {
        std::cerr << "subscribe failed.\n";
        return false;
    }

    int done = 0;
    while (!done) {
        if (redisBufferWrite(this->_subscribe_context, &done) == REDIS_ERR) {
            std::cerr << "subscribe failed.\n";
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel)
{
    if (redisAppendCommand(this->_subscribe_context, "UNSUBSCRIBE %d", channel) == REDIS_ERR) {
        std::cerr << "subscribe failed.\n";
        return false;
    }

    int done = 0;
    while (!done) {
        if (redisBufferWrite(this->_subscribe_context, &done) == REDIS_ERR) {
            std::cerr << "subscribe failed.\n";
            return false;
        }
    }
    return true;
}

void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    while (redisGetReply(this->_subscribe_context, (void**)&reply) == REDIS_OK) {
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
}

void Redis::init_notify_handler(std::function<void(int, std::string)> fn)
{
    this->_notify_message_handler = fn;
}