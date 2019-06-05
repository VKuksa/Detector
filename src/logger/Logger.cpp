#include "Logger.hpp"

#include "../Environment.h"

#include <fstream>
#include <iostream>

template<typename StreamType>
void Logger::init(StreamType stream) {
    /*auto format = expr::stream << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S") << ": <" << logging::trivial::severity
                               << "> " << expr::smessage;

    logging::add_file_log(keywords::file_name = Environment::rootDir() / "logs" / "log_%N.log",
                          keywords::rotation_size = 10 * 1024 * 1024,
                          keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
                          keywords::format = format);

    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);

    logging::add_common_attributes();
    logging::add_console_log(std::cout, keywords::format = format);*/
}
