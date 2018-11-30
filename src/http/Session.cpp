//
// Created by vkuksa on 29.11.2018.
//

#include "Session.h"


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace {
    using namespace boost::beast::http;

    boost::beast::string_view mime_type(boost::beast::string_view path)
    {
        using boost::beast::iequals;
        auto const ext = [&path]
        {
            auto const pos = path.rfind(".");
            if(pos == boost::beast::string_view::npos)
                return boost::beast::string_view{};
            return path.substr(pos);
        }();
        if(iequals(ext, ".htm"))  return "text/html";
        if(iequals(ext, ".html")) return "text/html";
        if(iequals(ext, ".php"))  return "text/html";
        if(iequals(ext, ".css"))  return "text/css";
        if(iequals(ext, ".txt"))  return "text/plain";
        if(iequals(ext, ".js"))   return "application/javascript";
        if(iequals(ext, ".json")) return "application/json";
        if(iequals(ext, ".xml"))  return "application/xml";
        if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
        if(iequals(ext, ".flv"))  return "video/x-flv";
        if(iequals(ext, ".png"))  return "image/png";
        if(iequals(ext, ".jpe"))  return "image/jpeg";
        if(iequals(ext, ".jpeg")) return "image/jpeg";
        if(iequals(ext, ".jpg"))  return "image/jpeg";
        if(iequals(ext, ".gif"))  return "image/gif";
        if(iequals(ext, ".bmp"))  return "image/bmp";
        if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
        if(iequals(ext, ".tiff")) return "image/tiff";
        if(iequals(ext, ".tif"))  return "image/tiff";
        if(iequals(ext, ".svg"))  return "image/svg+xml";
        if(iequals(ext, ".svgz")) return "image/svg+xml";
        return "application/text";
    }

    std::string
    path_cat(
            boost::beast::string_view base,
            boost::beast::string_view path)
    {
        if(base.empty())
            return path.to_string();
        std::string result = base.to_string();
#if BOOST_MSVC
        char constexpr path_separator = '\\';
        if(result.back() == path_separator)
            result.resize(result.size() - 1);
        result.append(path.data(), path.size());
        for(auto& c : result)
            if(c == '/')
                c = path_separator;
#else
        char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
        return result;
    }

    template<
            class Body, class Allocator,
            class Send>
    void
    handle_request(
            Send&& send,
            request<Body, basic_fields<Allocator>>&& req,
            boost::beast::string_view doc_root = "..\\third_party")
    {
        // Returns a bad request response
        auto const bad_request =
                [&req](boost::beast::string_view why)
                {
                   response<string_body> res{status::bad_request, req.version()};
                    res.set(field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = why.to_string();
                    res.prepare_payload();
                    return res;
                };

        // Returns a not found response
        auto const not_found =
                [&req](boost::beast::string_view target)
                {
                    response<string_body> res{status::not_found, req.version()};
                    res.set(field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = "The resource '" + target.to_string() + "' was not found.";
                    res.prepare_payload();
                    return res;
                };

        // Returns a server error response
        auto const server_error =
                [&req](boost::beast::string_view what)
                {
                    response<string_body> res{status::internal_server_error, req.version()};
                    res.set(field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = "An error occurred: '" + what.to_string() + "'";
                    res.prepare_payload();
                    return res;
                };

        // Make sure we can handle the method
        if( req.method() != verb::get &&
            req.method() != verb::head)
            return send(bad_request("Unknown HTTP-method"));

        // Request path must be absolute and not contain "..".
        if( req.target().empty() ||
            req.target()[0] != '/' ||
            req.target().find("..") != boost::beast::string_view::npos)
            return send(bad_request("Illegal request-target"));

        // Build the path to the requested file
        std::string path = path_cat(doc_root, req.target());
        if(req.target().back() == '/')
            path.append("index.html");

        // Attempt to open the file
        boost::beast::error_code ec;
        file_body::value_type body;
        body.open(path.c_str(), boost::beast::file_mode::scan, ec);

        // Handle the case where the file doesn't exist
        if(ec == boost::system::errc::no_such_file_or_directory)
            return send(not_found(req.target()));

        // Handle an unknown error
        if(ec)
            return send(server_error(ec.message()));

        // Cache the size since we need it after the move
        auto const size = body.size();

        // Respond to HEAD request
        if(req.method() == verb::head)
        {
            response<empty_body> res{status::ok, req.version()};
            res.set(field::server, BOOST_BEAST_VERSION_STRING);
            res.set(field::content_type, mime_type(path));
            res.content_length(size);
            res.keep_alive(req.keep_alive());
            return send(std::move(res));
        }

        // Respond to GET request
        response<file_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(status::ok, req.version())};
        res.set(field::server, BOOST_BEAST_VERSION_STRING);
        res.set(field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }
}

namespace http {

    Session::Session(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)),
                                                            strand_(socket_.get_executor()),
															lambda_(*this){

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
        handle_request(lambda_, std::move(req_) ); //TODO refactor request handling
    }

    void Session::onWrite(boost::system::error_code ec, std::size_t bytes_transferred, bool close) {
        boost::ignore_unused(bytes_transferred);

        if(close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return doClose();
        }
        res_ = nullptr;

        // Read another request
        doRead();
    }

    void Session::doClose() {
        boost::system::error_code ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    }
}