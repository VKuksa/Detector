//
// Created by vkuksa on 29.11.2018.
//

#ifndef DETECTOR_SERVER_H
#define DETECTOR_SERVER_H

#include <boost/asio/ip/tcp.hpp>

namespace http {
    class Server {
    public:
        Server(boost::asio::io_context &ioc, uint16_t port);

        void run();

        void doAccept();

    private:
        boost::asio::ip::tcp::acceptor acceptor_;
        boost::asio::ip::tcp::socket socket_;
    };
}

#endif //DETECTOR_SERVER_H
