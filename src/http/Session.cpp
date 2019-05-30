#include "Session.h"

#include "../Environment.h"

#include <boost/asio/bind_executor.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/log/trivial.hpp>
#include <thread>

namespace {
    using namespace boost::beast::http;

    boost::beast::string_view mime_type(boost::beast::string_view && path) {
        using boost::beast::iequals;

        auto const ext = [&path] {
            auto const pos = path.rfind(".");
            if (pos == boost::beast::string_view::npos)
                return boost::beast::string_view{};
            return path.substr(pos);
        }();

        if (iequals(ext, ".htm"))
            return "text/html";
        if (iequals(ext, ".html"))
            return "text/html";
        if (iequals(ext, ".php"))
            return "text/html";
        if (iequals(ext, ".css"))
            return "text/css";
        if (iequals(ext, ".txt"))
            return "text/plain";
        if (iequals(ext, ".js"))
            return "application/javascript";
        if (iequals(ext, ".json"))
            return "application/json";
        if (iequals(ext, ".xml"))
            return "application/xml";
        if (iequals(ext, ".swf"))
            return "application/x-shockwave-flash";
        if (iequals(ext, ".flv"))
            return "video/x-flv";
        if (iequals(ext, ".png"))
            return "image/png";
        if (iequals(ext, ".jpe"))
            return "image/jpeg";
        if (iequals(ext, ".jpeg"))
            return "image/jpeg";
        if (iequals(ext, ".jpg"))
            return "image/jpeg";
        if (iequals(ext, ".gif"))
            return "image/gif";
        if (iequals(ext, ".bmp"))
            return "image/bmp";
        if (iequals(ext, ".ico"))
            return "image/vnd.microsoft.icon";
        if (iequals(ext, ".tiff"))
            return "image/tiff";
        if (iequals(ext, ".tif"))
            return "image/tiff";
        if (iequals(ext, ".svg"))
            return "image/svg+xml";
        if (iequals(ext, ".svgz"))
            return "image/svg+xml";
        return "application/text";
    }

    template<class Body, class Allocator, class Send>
    void handle_request(Send && send, request<Body, basic_fields<Allocator>> && req, boost::beast::string_view doc_root = "..\\third_party") {
        auto const bad_request = [&req](boost::beast::string_view why) {
            response<string_body> res{status::bad_request, req.version()};
            res.set(field::server, BOOST_BEAST_VERSION_STRING);
            res.set(field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = why.to_string();
            res.prepare_payload();
            return res;
        };

        auto const not_found = [&req](boost::beast::string_view target) {
            response<string_body> res{status::not_found, req.version()};
            res.set(field::server, BOOST_BEAST_VERSION_STRING);
            res.set(field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "The resource '" + target.to_string() + "' was not found.";
            res.prepare_payload();
            return res;
        };

        auto const server_error = [&req](boost::beast::string_view what) {
            response<string_body> res{status::internal_server_error, req.version()};
            res.set(field::server, BOOST_BEAST_VERSION_STRING);
            res.set(field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "An error occurred: '" + what.to_string() + "'";
            res.prepare_payload();
            return res;
        };

        if (req.method() != verb::get && req.method() != verb::head)
            return send(bad_request("Unknown HTTP-method"));

        if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos)
            return send(bad_request("Illegal request-target"));

        auto path = Environment::resourcesDir() / "index.html";

        boost::beast::error_code ec;
        file_body::value_type body;
        body.open(path.string().c_str(), boost::beast::file_mode::scan, ec);

        if (ec == boost::system::errc::no_such_file_or_directory)
            return send(not_found(req.target()));

        if (ec)
            return send(server_error(ec.message()));

        auto const size = body.size();

        if (req.method() == verb::head) {
            response<empty_body> res{status::ok, req.version()};
            res.set(field::server, BOOST_BEAST_VERSION_STRING);
            res.set(field::content_type, mime_type(path.string()));
            res.content_length(size);
            res.keep_alive(req.keep_alive());
            return send(std::move(res));
        }

        response<file_body> res{std::piecewise_construct, std::make_tuple(std::move(body)), std::make_tuple(status::ok, req.version())};
        res.set(field::server, BOOST_BEAST_VERSION_STRING);
        res.set(field::content_type, mime_type(path.string()));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }
}

namespace http {
    Session::Session(boost::asio::ip::tcp::socket socket)
        : socket_(std::move(socket))
        , strand_(socket_.get_executor())
        , lambda_(*this) {}

    void Session::run() {
        doRead();
    }

    void Session::doRead() {
        async_read(socket_,
                   buffer_,
                   req_,
                   boost::asio::bind_executor(strand_, std::bind(&Session::onRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
    }

    void Session::onRead(boost::system::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec == error::end_of_stream)
            return doClose();

        handle_request(lambda_, std::move(req_)); //TODO refactor request handling
    }

    void Session::onWrite(boost::system::error_code ec, std::size_t bytes_transferred, bool close) {
        boost::ignore_unused(bytes_transferred);

        if (close) {
            return doClose();
        }
        res_ = nullptr;

        doRead();
    }

    void Session::doClose() {
        boost::system::error_code ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    }
}
