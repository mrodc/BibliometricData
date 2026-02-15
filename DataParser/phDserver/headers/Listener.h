//
// Created by mrod on 6/20/25.
//

#ifndef LISTENER_H
#define LISTENER_H
#include "globalHeaders.h"
#include "Session.h"

class Listener : public std::enable_shared_from_this<Listener>{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    Listener(
        net::io_context& ioc,
        tcp::endpoint endpoint);

    void run();

private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
    void fail(beast::error_code ec, char const* what);



};



#endif //LISTENER_H
