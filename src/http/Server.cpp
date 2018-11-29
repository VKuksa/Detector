//
// Created by vkuksa on 29.11.2018.
//

#include "Server.h"
#include "Session.h"

#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>

namespace {
    namespace asio = boost::asio;
    namespace ip = asio::ip;

    void fail(boost::system::error_code ec, char const *what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }
}

namespace http {
    Server::Server(asio::io_context &ioc, uint16_t port) : acceptor_(ioc,
                                                                     ip::tcp::endpoint(ip::make_address_v4("0.0.0.0"),
                                                                                       port)),
                                                           socket_(ioc) {
        boost::system::error_code ec;
        ip::tcp::endpoint endpoint;

        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            fail(ec, "open");
            return;
        }

        acceptor_.set_option(boost::asio::socket_base::reuse_address(true));

        acceptor_.bind(endpoint, ec);
        if (ec) {
            fail(ec, "bind");
            return;
        }

        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec) {
            fail(ec, "listen");
            return;
        }
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
                std::make_shared<Session>(std::move(socket_))->run();
            }

            doAccept();
        });
    }
}