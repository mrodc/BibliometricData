//
// Created by mrodc on 9/24/25.
//

#ifndef SERVERSOCKET_USERTIMER_H
#define SERVERSOCKET_USERTIMER_H
#include "globalHeaders.h"


class UserTimer
{
public:
    UserTimer(boost::asio::io_context& io_context, std::chrono::steady_clock::duration interval);

    ~UserTimer();
    std::thread *io_thread;

private:
    void start_wait();
    boost::asio::steady_timer *timer_;
    std::chrono::steady_clock::duration interval_;
};
#endif
//SERVERSOCKET_USERTIMER_H