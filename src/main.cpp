#include "Environment.h"
#include "http/Server.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/program_options.hpp>
#include <iostream>

namespace {
    void initLogger() {
        namespace logging = boost::log;
        namespace src = boost::log::sources;
        namespace keywords = boost::log::keywords;
        namespace sinks = boost::log::sinks;
        namespace expr = boost::log::expressions;

        auto format = expr::stream << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S") << ": <" << logging::trivial::severity
                                   << "> " << expr::smessage;

        logging::add_file_log(keywords::file_name = Environment::rootDir() / "logs" / "log_%N.log",
                              keywords::rotation_size = 10 * 1024 * 1024,
                              keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
                              keywords::format = format);

        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);

        logging::add_common_attributes();
        logging::add_console_log(std::cout, keywords::format = format);
    }

}

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    uint16_t httpPort;
    po::options_description desc("Allowed options");
    desc.add_options()("help, h", "produce help message")("port, p", po::value<uint16_t>(&httpPort)->default_value(8888), "http server port");

    po::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
        std::cout << "This application is the backend of Detector app"
                  << "\n";
        return EXIT_FAILURE;
    }

    if (vm.count("port")) {
        std::cout << "Http server will run on " << vm["port"].as<uint16_t>() << " port.\n";
    }

    try {
        boost::asio::io_context ioc;
        http::Server server(ioc, httpPort);

        server.run();
        ioc.run();
    } catch (std::exception &exc) {
        std::cout << exc.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
