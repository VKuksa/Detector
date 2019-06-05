#pragma once
#include <chrono>
#include <ostream>
#include <sstream>

enum Severity { trace, info, warning, error, critical, none };

class Logger {
public:
    template<Severity severity>
    class Record {
    public:
        Record(Record const &) = delete;

        Record & operator=(Record const &) = delete;

        std::ostringstream & create() {
            buffer_ << "- " << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            buffer_ << " <" << ToString(level) << ">: "; //TODO
            return buffer_;
        }

        ~Record() {
            if (severity >= threshold_) {
                buffer_ << std::endl;
                *output_ << buffer_.str().c_str();
                output_->flush();
            }
        }

    private:
        Record(Record &&) = default;

        Record & operator=(Record &&) = default;

        std::ostringstream buffer_;
    };

    Logger(Logger const &) = delete;
    Logger(Logger &&) = delete;

    Logger & operator=(Logger const &) = delete;
    Logger & operator=(Logger &&) = delete;

    template<typename StreamType>
    static void init(StreamType stream);

    template<typename Severity>
    auto log() const {
        return Record<Severity>();
    }

private:
    static std::unique_ptr<std::ostream> output_;

    inline static Severity threshold_{none};
};
