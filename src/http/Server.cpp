#include "Server.h"

#include "../logger/Logger.hpp"
#include "Session.h"

#include <boost/beast/websocket.hpp>

namespace {
    namespace asio = boost::asio;
    namespace ip = asio::ip;
}

namespace http {
    Server::Server(asio::io_context & ioc, uint16_t port)
        : acceptor_(ioc, ip::tcp::endpoint(ip::make_address_v4("0.0.0.0"), port))
        , socket_(ioc) {
        logging::Logger::INFO() << "Server created. Port:" << std::to_string(port);
    }

    void Server::run() {
        doAccept();
    }

    void Server::doAccept() {
        if (!acceptor_.is_open()) {
            logging::Logger::INFO() << "Acceptor has not opened";
            return;
        }

        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                logging::Logger::INFO() << "Accepted connection";
                std::make_shared<Session>(std::move(socket_))->run();
            }

            doAccept();
        });
    }
}
