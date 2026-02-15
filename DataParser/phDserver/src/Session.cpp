//
// Created by mrod on 6/20/25.
//

#include "Session.h"


Session::Session(tcp::socket &&socket) : ws_(std::move(socket))
{
    rxComplete = "";

    stopTimer = false;

    interval_  = (1s);
    timer_ = new boost::asio::steady_timer (io_context, 1s);
    // Start the first asynchronous wait
    start_wait();
    io_thread = new std::thread ([&]() {
        io_context.run();
    });

}

Session::~Session() {
    stopTimer = true;
    timer_->cancel();

    io_context.stop();
    io_thread->join();
    free(io_thread);
    free(timer_);
}

void Session::run() {
    // Set a decorator to change the Server header of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
        {
            res.set(http::field::server,
                std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
        }));

    // Accept the websocket handshake
    ws_.async_accept(
        beast::bind_front_handler(
            &Session::on_accept,
            shared_from_this()));
}

void Session::on_accept(beast::error_code ec) {
    if (ec)
        return fail(ec, "accept");
    auto tempBuffer_ = fill_buffer("Hello!!\nConnection Ready from local server...");

    ws_.write(
        tempBuffer_.data()
        );

    // Read a message
    do_read();
}

void Session::do_read() {
    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &Session::on_read,
            shared_from_this()));
}

void Session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    string ENDSTR = "<|/|>";

    if (ec == websocket::error::closed) {
        io_context.stop();
        return; // Client closed the connection
    }

    if (ec)
        return fail(ec, "read");

    const std::string rxMsg(boost::asio::buffers_begin(buffer_.data()), boost::asio::buffers_end(buffer_.data()));
    bool match = false;
    string posFix = "";
    string preFix = split(rxMsg, ENDSTR, &posFix, &match, FULL_LIMITER);
    if ( match ) {
        rxComplete += preFix;
        const std::string reply = fnc.rxMessage(rxComplete);
        data_ = fill_buffer(reply);
        rxComplete = posFix;
    }
    else {
        rxComplete += preFix;
        json nextData;
        nextData["action"] = "nextData";//rdJson["action"];
        nextData["data"] = "nextData";
        string nxt = nextData.dump();
        data_ = fill_buffer(nxt);
    }


    ws_.async_write(
    data_.data(),
    beast::bind_front_handler(
        &Session::on_write,
        shared_from_this()));


    // const std::string reply = fnc.rxMessage(rxMsg);
    //
    // data_ = fill_buffer(reply);
    //
    // ws_.async_write(
    //     data_.data(),
    //     beast::bind_front_handler(
    //         &Session::on_write,
    //         shared_from_this()));
}
//dimensionsData collection provided 5259 records for crossrefData
//5717 - 5259 = 455 records provided by ieeeData collection
void Session::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec)
        return fail(ec, "write");
    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    do_read();

}

void Session::fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}


beast::flat_buffer Session::fill_buffer(string data) {
    beast::flat_buffer tempBuffer_;
    //std::string received_data = data;
    tempBuffer_.prepare(data.size());
    std::memcpy(tempBuffer_.data().data(), data.data(), data.size());
    tempBuffer_.commit(data.size());

    return tempBuffer_;
}

void Session::start_wait() {
    if (stopTimer) {
        timer_->cancel();
        return;
    }
    timer_->async_wait(
    [this](const boost::system::error_code& error)
    {
        if (!error)
        {
            boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
             //std::cout << "ISO string: " << boost::posix_time::to_iso_string(now) << std::endl;

            string dtime = boost::posix_time::to_simple_string(now);
            std::vector<std::string> tokens;

            // Split the string by the dot delimiter
            boost::algorithm::split(tokens, dtime, boost::is_any_of("."));

             //std::cout << "Simple string: " << tokens[0] << std::endl;


            json message;
            message["Action"] = "dateTime";
            message["Data"] = tokens[0];
            string reply = message.dump();

            auto dateMsg = fill_buffer(reply);
            ws_.write(dateMsg.data());

             // Schedule the next wait by setting a new expiry time
            timer_->expires_at(timer_->expiry() + interval_);
            start_wait(); // Re-initiate the async wait
        }
        else if (error != boost::asio::error::operation_aborted)
        {
            // Handle other errors (e.g., timer cancellation)
            std::cerr << "Timer error: " << error.message() << "\n";
        }
    });
}

std::string Session::split(string text, string delim,string *posFix, bool *foundOK, bool FullDelimeter) {

    string preFix = "";
    *posFix = "";
    // std::vector<std::string> results;
    // // Define the regular expression for the delimiter.
    // // Use boost::regex for the pattern matching.
    // boost::regex delimiter_regex(delim);
    // boost::algorithm::split_regex(results, text, delimiter_regex);

    string lookFor = regexEscape(delim);
    boost::regex pattern("(.*)\\s*(" + lookFor +")\\s*(.*)");
    boost::smatch results;
    if (boost::regex_search(text, results, pattern)) {
        *foundOK = true;
        // // The overall match (index 0)
        // std::cout << "Full match: " << results[0] << std::endl;
        //
        // // Marked sub-expressions (capture groups)
        // std::cout << "Group 1: " << results[1] << std::endl; // Anything before "regular"
        // std::cout << "Group 2: " << results[2] << std::endl; // "regular"
        // std::cout << "Group 3: " << results[3] << std::endl; // Anything after "regular"
        *posFix = results[3];
        preFix = results[1];
        // // sub_match objects can be implicitly converted to std::string
        // std::string group3_str = results[3];
        // std::cout << "Group 3 as string: " << group3_str << std::endl;
    } else {
        *foundOK = false;
        preFix = text;
        std::cout << "No match found." << std::endl;
    }

    return preFix;

}
string Session::regexEscape( string word){
    const boost::regex esc("[.^$|()\\[\\]{}*+?\\\\]");
    const std::string rep("\\\\&");
    return regex_replace(word, esc, rep,
                         boost::match_default | boost::format_sed);


}

