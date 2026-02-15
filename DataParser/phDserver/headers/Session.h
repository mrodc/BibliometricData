//
// Created by mrod on 6/20/25.
//

#ifndef SESSION_H
#define SESSION_H
#include "globalHeaders.h"
#include  "UserTimer.h"
#include "ServerFunction.h"


class Session : public std::enable_shared_from_this<Session>
{
    // websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    beast::flat_buffer data_;
public:
    string rxComplete;
    bool stopTimer;
    net::io_context iocS;
    // Take ownership of the socket
    explicit Session(tcp::socket&& socket) ;
    ~Session();

    void run();
    void on_accept(beast::error_code ec);
    void do_read();

    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    void fail(beast::error_code ec, char const* what);

    string split(string s, string delim, string *postFix, bool *foundOK, bool FullDelimeter = false);
    string regexEscape( string word);
    websocket::stream<beast::tcp_stream> ws_;

private:
    boost::asio::io_context io_context;
    ServerFunction fnc;
    beast::flat_buffer fill_buffer(string data);
    std::thread *io_thread;


    void start_wait();
    boost::asio::steady_timer *timer_;
    std::chrono::steady_clock::duration interval_;


};


#endif //SESSION_H
