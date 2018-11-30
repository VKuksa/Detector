//
// Created by vkuksa on 29.11.2018.
//

#include "Server.h"
#include "Session.h"

#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>

namespace {
    namespace asio = boost::asio;
    namespace ip = asio::ip;
}

namespace http {
    Server::Server(asio::io_context &ioc, uint16_t port) : acceptor_(ioc,
                                                                     ip::tcp::endpoint(ip::make_address_v4("0.0.0.0"),
                                                                                       port)),
                                                           socket_(ioc) {
    }

    void Server::run() {
        if (!acceptor_.is_open())
            return;
        doAccept();
    }

    void Server::doAccept() {
        if (!acceptor_.is_open()) {
            return;
        }

        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                std::cout << "Accepted and created session" << std::endl;
                std::make_shared<Session>(std::move(socket_))->run();
            }

            doAccept();
        });
    }
}