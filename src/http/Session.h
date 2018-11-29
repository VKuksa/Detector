//
// Created by vkuksa on 29.11.2018.
//

#ifndef DETECTOR_SESSION_H
#define DETECTOR_SESSION_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/core.hpp>
#include <memory>


namespace http {
    class Session : public std::enable_shared_from_this<Session> {
    public:
        explicit Session(boost::asio::ip::tcp::socket socket);

        void run();

    private:
        void doRead();

        void onRead(boost::system::error_code ec, std::size_t bytes_transferred);

        void onWrite(boost::system::error_code ec, std::size_t bytes_transferred, bool close);

        void doClose();

        boost::asio::ip::tcp::socket socket_;
        boost::asio::strand<boost::asio::io_context::executor_type> strand_;
        boost::beast::http::request<boost::beast::http::string_body> req_{};
        boost::beast::flat_buffer buffer_;
    };
}

#endif //DETECTOR_SESSION_H
