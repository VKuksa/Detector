#pragma once

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
