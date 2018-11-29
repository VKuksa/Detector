//
// Created by vkuksa on 29.11.2018.
//

#include "Session.h"


#include <boost/beast/http.hpp>
#include <boost/asio/bind_executor.hpp>
#include <iostream>
#include <thread>

namespace {
    using namespace boost::beast::http;
}

namespace http {

    Session::Session(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)),
                                                            strand_(socket_.get_executor()) {

    }

    void Session::run() {
        doRead();
    }

    void Session::doRead() {
        async_read(socket_, buffer_, req_,
                   boost::asio::bind_executor(
                           strand_,
                           std::bind(
                                   &Session::onRead,
                                   shared_from_this(),
                                   std::placeholders::_1,
                                   std::placeholders::_2)));
    }

    void Session::onRead(boost::system::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == error::end_of_stream)
            return doClose();

        // Send the response
        //handle_request(*doc_root_, std::move(req_), lambda_); //TODO refactor request handling
    }

    void Session::onWrite(boost::system::error_code ec, std::size_t bytes_transferred, bool close) {
        boost::ignore_unused(bytes_transferred);

        if(close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return doClose();
        }

        // Read another request
        doRead();
    }

    void Session::doClose() {
        boost::system::error_code ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    }
}