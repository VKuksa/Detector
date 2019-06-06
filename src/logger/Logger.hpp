#pragma once
#include <ostream>
#include <sstream>

namespace logging {
    enum Severity { trace, debug, info, warning, error, critical };

    class Logger {
    public:
        class Record {
        public:
            friend Logger;

            Record(Record const &) = delete;
            Record & operator=(Record const &) = delete;

            Record(Record &&) = default;
            Record & operator=(Record &&) = default;

            ~Record();

            template<class T>
            Record & operator<<(const T & t);

        private:
            Record(Severity severity);

            inline static Severity threshold_{trace};

            inline static bool verboseLogging_{false};

            Severity severity_;

            std::unique_ptr<std::ostringstream> stream_;
        };

        enum class Mode { Ordinary, Verbose };

        Logger(Logger const &) = delete;
        Logger(Logger &&) = delete;

        Logger & operator=(Logger const &) = delete;
        Logger & operator=(Logger &&) = delete;

        static Logger & instance();

        static void init();

        static Record TRACE();

        static Record DEBUG();

        static Record INFO();

        static Record WARN();

        static Record ERR();

        static Record CRITICAL();

    protected:
        Logger() = default;
        ~Logger() = default;

        std::unique_ptr<std::ostream> output_;
    };
}

#include "Logger.ipp"
