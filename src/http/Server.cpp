#include "Server.h"

#include "Session.h"

#include <boost/beast/websocket.hpp>
#include <boost/log/trivial.hpp>

namespace {
    namespace asio = boost::asio;
    namespace ip = asio::ip;
}

namespace http {
    Server::Server(asio::io_context & ioc, uint16_t port)
        : acceptor_(ioc, ip::tcp::endpoint(ip::make_address_v4("0.0.0.0"), port))
        , socket_(ioc) {
        BOOST_LOG_TRIVIAL(info) << "Server created. Port:" << std::to_string(port);
    }

    void Server::run() {
        doAccept();
    }

    void Server::doAccept() {
        if (!acceptor_.is_open()) {
            BOOST_LOG_TRIVIAL(error) << "Acceptor has not opened";
            return;
        }

        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                BOOST_LOG_TRIVIAL(info) << "Accepted connection";
                std::make_shared<Session>(std::move(socket_))->run();
            }

            doAccept();
        });
    }
}
