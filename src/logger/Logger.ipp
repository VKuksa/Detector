#include "../Environment.h"

#include <fstream>
#include <iostream>
#include <chrono>
#include <string>

namespace logging {
    template<class T>
	Logger::Record& Logger::Record::operator<<(const T& t) {
		if(stream_) {
			using sc = std::chrono::system_clock; 
			auto time = sc::to_time_t(sc::now());
			std::string ts = std::ctime(&time);
			ts.resize(ts.size()-1);

			std::string severity;
			switch(severity_) {
			    case 0 : 
					severity = "trace"; 
					break;       
			    case 1 : 
					severity = "debug"; 
				    break;
				case 2 : 
					severity = "info"; 
				    break;
				case 3 : 
					severity = "warning"; 
				    break;
				case 4 : 
					severity = "error"; 
				    break;
				case 5 : 
					severity = "critical"; 
				    break;
                default:
					severity = "unknown";
                }

		    *stream_ << "[" << ts;
		    *stream_ << "]" << " <" << severity << ">: "; 
			*stream_ << t;
		}
	    return *this;
	}

	inline Logger::Record::Record(Severity severity) : severity_(severity), stream_(std::make_unique<std::ostringstream>())	{}

	inline Logger::Record::~Record() {
	    if (severity_ >= threshold_ && stream_) {
	        *stream_ << std::endl;
			auto message = stream_->str();

			std::cout << message;
			std::cout.flush();

			*instance().output_ << message;
			 instance().output_->flush();
	    }
	}

	inline Logger& Logger::instance() {
		static Logger instance;		//thread-safe since c++11, no sync methods needed
		return instance;
	}

	inline void Logger::init() {	//TODO: May be refactored
		instance().output_ = std::make_unique<std::ofstream>(Environment::rootDir() / "logs" / "detector.log");
		Record::threshold_ = debug;
    }

    inline Logger::Record Logger::TRACE() {
		return Record(trace);
	}

	inline Logger::Record Logger::DEBUG() {
		return Record(debug);
	}

	inline Logger::Record Logger::INFO() {
		return Record(info);
	}

	inline Logger::Record Logger::WARN() {
		return Record(warning);
	}

	inline Logger::Record Logger::ERR() {
		return Record(error);
	}

	inline Logger::Record Logger::CRITICAL() {
		return Record(critical);
	}
}
