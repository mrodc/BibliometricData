//
// Created by mrod on 6/20/25.
//
// This function produces an error message from an error_code

#include "globalHeaders.h"
// #include "Session.h"
#include "Listener.h"
// #include "../headers/PocoTimer.h"

void runWsocket() {
    auto const address = net::ip::make_address("0.0.0.0");
    //auto const port = static_cast<unsigned short>(std::atoi("3349"));
    auto const port = static_cast<unsigned short>(std::atoi("9055"));

    // The io_context is required for all I/O
    net::io_context ioc{1};

    // Create and launch a listening port
    std::make_shared<Listener>(ioc, tcp::endpoint{address, port})->run();

    cout << "Listening... Waiting for connections..." << endl;

    // Run the I/O service on the requested number of threads
    ioc.run();


}

int main(int argc, char* argv[])
{


    //rapidJson();
    //openCV();
    //runPocoTimer();
    runWsocket();





    return EXIT_SUCCESS;
}