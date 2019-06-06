#include "http/Server.h"
#include "logger/Logger.hpp"

#include <boost/program_options.hpp>
#include <iostream>

int main(int argc, char ** argv) {
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
        logging::Logger::init();

        logging::Logger::INFO() << "Application started";

        boost::asio::io_context ioc;
        http::Server server(ioc, httpPort);

        server.run();
        ioc.run();
    } catch (std::exception & exc) {
        std::cout << exc.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
