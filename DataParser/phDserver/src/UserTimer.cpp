//
// Created by mrodc on 9/24/25.
//

#include "UserTimer.h"

UserTimer::UserTimer(boost::asio::io_context& io, std::chrono::steady_clock::duration interval)
        // : timer_(io, interval), interval_(interval)
{
    interval_  = (interval);
    timer_ = new boost::asio::steady_timer (io, interval);
    // Start the first asynchronous wait
    start_wait();

    io_thread = new std::thread ([&io]() {
            io.run();
        });

}

UserTimer::~UserTimer() {


    // io_thread->detach();
    io_thread->join();
    free(timer_);

}

void UserTimer::start_wait()
{
    timer_->async_wait(
        [this](const boost::system::error_code& error)
        {
            if (!error)
            {
                // Timer expired, perform the desired action
                std::cout << "Timer expired! Current time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() << "ms\n";

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
