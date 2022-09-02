#include "parallelzone/logger.hpp"
#include <fstream>
#include <iostream>
namespace parallelzone {

namespace detail_ {
class LoggerPIMPL {
    using my_type = LoggerPIMPL;
    using my_ptr  = std::unique_ptr<my_type>;

public:
    LoggerPIMPL()                   = default;
    virtual ~LoggerPIMPL() noexcept = default;

    virtual my_ptr clone() const   = 0;
    virtual std::ostream& stream() = 0;
};
} // namespace detail_

Logger::Logger() : Logger(nullptr) {}
Logger::Logger(pimpl_ptr&& p) : m_pimpl_(std::move(p)) {}
Logger::~Logger() noexcept = default;

Logger::Logger(Logger&&) noexcept = default;
Logger& Logger::operator=(Logger&&) noexcept = default;

#define ASSERT_PIMPL(p) \
    if(!p) throw std::runtime_error("LoggerPIMPL not set");

Logger::Logger(const Logger& other) :
  Logger(other.m_pimpl_ ? other.m_pimpl_->clone() : nullptr) {}

Logger& Logger::operator=(const Logger& other) {
    if(other.m_pimpl_) {
        m_pimpl_ = std::move(other.m_pimpl_->clone());
    } else {
        m_pimpl_ = nullptr;
    }
    return *this;
}

std::ostream& Logger::stream() {
    ASSERT_PIMPL(m_pimpl_)
    return m_pimpl_->stream();
}

Logger& Logger::print(const std::string& msg) {
    stream() << msg;
    return *this;
}

#undef ASSERT_PIMPL

class StdOutLogger : public detail_::LoggerPIMPL {
    using base_type = detail_::LoggerPIMPL;
    using base_ptr  = std::unique_ptr<base_type>;
    using my_type   = StdOutLogger;

public:
    StdOutLogger()                 = default;
    ~StdOutLogger() noexcept final = default;

    StdOutLogger(const my_type&)     = default;
    StdOutLogger(my_type&&) noexcept = default;
    StdOutLogger& operator=(const my_type&) = default;
    StdOutLogger& operator=(my_type&&) noexcept = default;

    std::ostream& stream() final { return std::cout; }
    base_ptr clone() const final { return std::make_unique<my_type>(); }
};

Logger make_stdout_logger() { return Logger(std::make_unique<StdOutLogger>()); }

class StdErrLogger : public detail_::LoggerPIMPL {
    using base_type = detail_::LoggerPIMPL;
    using base_ptr  = std::unique_ptr<base_type>;
    using my_type   = StdErrLogger;

public:
    StdErrLogger()                 = default;
    ~StdErrLogger() noexcept final = default;

    StdErrLogger(const my_type&)     = default;
    StdErrLogger(my_type&&) noexcept = default;
    StdErrLogger& operator=(const my_type&) = default;
    StdErrLogger& operator=(my_type&&) noexcept = default;

    std::ostream& stream() final { return std::cerr; }
    base_ptr clone() const final { return std::make_unique<my_type>(); }
};

Logger make_stderr_logger() { return Logger(std::make_unique<StdErrLogger>()); }

class FileLogger : public detail_::LoggerPIMPL {
    using base_type = detail_::LoggerPIMPL;
    using base_ptr  = std::unique_ptr<base_type>;
    using my_type   = FileLogger;

public:
    FileLogger() = delete;
    FileLogger(std::string fname) :
      m_fstream_(std::make_unique<std::ofstream>(fname)) {}

    virtual ~FileLogger() noexcept = default;

    FileLogger(const my_type&)     = delete;
    FileLogger(my_type&&) noexcept = default;
    FileLogger& operator=(const my_type&) = delete;
    FileLogger& operator=(my_type&&) noexcept = default;

    std::ostream& stream() final { return *m_fstream_; }
    base_ptr clone() const final {
        throw std::runtime_error("FileLogger cannot clone");
        return nullptr;
    }

protected:
    std::unique_ptr<std::ofstream> m_fstream_;
};

Logger make_file_logger(std::string fname) {
    return Logger(std::make_unique<FileLogger>(fname));
}

// TODO: This can probbaly be made more performance by adding
// a custom extension of std::streambuf that has a nullcall in
// xputs / overflow
class NullLogger : public FileLogger {
    using my_type = NullLogger;

public:
    NullLogger() : FileLogger("/dev/null"){};
    ~NullLogger() noexcept final = default;

    NullLogger(const my_type&)     = default;
    NullLogger(my_type&&) noexcept = default;
    NullLogger& operator=(const my_type&) = default;
    NullLogger& operator=(my_type&&) noexcept = default;
};

Logger make_null_logger() { return Logger(std::make_unique<NullLogger>()); }

class StreamLogger : public detail_::LoggerPIMPL {
    using base_type = detail_::LoggerPIMPL;
    using base_ptr  = std::unique_ptr<base_type>;
    using my_type   = StreamLogger;

public:
    StreamLogger() = delete;
    StreamLogger(std::ostream* p) : m_stream_(p) {}
    ~StreamLogger() noexcept final = default;

    StreamLogger(const my_type&)     = default;
    StreamLogger(my_type&&) noexcept = default;
    StreamLogger& operator=(const my_type&) = default;
    StreamLogger& operator=(my_type&&) noexcept = default;

    std::ostream& stream() final {
        if(!m_stream_) throw std::runtime_error("Null Stream");
        return *m_stream_;
    }
    base_ptr clone() const final {
        return std::make_unique<my_type>(m_stream_);
    }

private:
    std::ostream* m_stream_ = nullptr;
};

Logger make_stream_logger(std::ostream* s) {
    return Logger(std::make_unique<StreamLogger>(s));
}

} // namespace parallelzone
