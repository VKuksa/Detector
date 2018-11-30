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
        struct send_lambda {
            Session &self_;

            explicit
            send_lambda(Session &self)
                    : self_(self) {
            }

            template<bool isRequest, class Body, class Fields>
            void operator()(boost::beast::http::message<isRequest, Body, Fields> &&msg) const {
                // The lifetime of the message has to extend
                // for the duration of the async operation so
                // we use a shared_ptr to manage it.
                auto sp = std::make_shared<
                        boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));

                // Store a type-erased version of the shared
                // pointer in the class to keep it alive.
                self_.res_ = sp;

                // Write the response
                boost::beast::http::async_write(
                        self_.socket_,
                        *sp,
                        boost::asio::bind_executor(
                                self_.strand_,
                                std::bind(
                                        &Session::onWrite,
                                        self_.shared_from_this(),
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        sp->need_eof())));
            }
        };

        void doRead();

        void onRead(boost::system::error_code ec, std::size_t bytes_transferred);

        void onWrite(boost::system::error_code ec, std::size_t bytes_transferred, bool close);

        void doClose();

        boost::asio::ip::tcp::socket socket_;
        boost::asio::strand<boost::asio::io_context::executor_type> strand_;
        boost::beast::http::request<boost::beast::http::string_body> req_{};
        std::shared_ptr<void> res_;
        boost::beast::flat_buffer buffer_;
        send_lambda lambda_;
    };
}

#endif //DETECTOR_SESSION_H
