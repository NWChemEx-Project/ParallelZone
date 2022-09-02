#pragma once
#include <memory>
#include <ostream>
#include <string>

namespace parallelzone {
namespace detail_ {
/// Logger Implemenetation Class
class LoggerPIMPL;
} // namespace detail_

/**
 *  A class to manage various stream loggers under a unified API
 */
class Logger {
    using pimpl_type = detail_::LoggerPIMPL;
    using pimpl_ptr  = std::unique_ptr<pimpl_type>;

public:
    /// Construct a Logger instance without implementation
    Logger();

    /**
     * @brief Construct a Logger instance from an implementation
     * pointer.
     *
     * @param p Implementation pointer
     * */
    Logger(pimpl_ptr&& p);

    /// Destruct Logger instance
    ~Logger() noexcept;

    /**
     * @brief Construct Logger instance via copy.
     *
     * @param[in] other Logger instance from which to construct
     *                  this instance.
     */
    Logger(const Logger& other);

    /**
     * @brief Construct Logger instance via move.
     *
     * @param[in] other Logger instance from which to construct
     *                  this instance.
     */
    Logger(Logger&& other) noexcept;

    /**
     * @brief Copy contents of Logger instance to this instance.
     *
     * @param[in] other Logger instance from which to copy
     * @returns   Reference to this instance
     */
    Logger& operator=(const Logger& other);

    /**
     * @brief Move contents of Logger instance to this instance.
     *
     * @param[in] other Logger instance from which to move
     * @returns   Reference to this instance
     */
    Logger& operator=(Logger&& other) noexcept;

    /**
     * @brief Obtain stream from underlying Logger instance.
     *
     * @returns Refeference to underlying stream instance
     */
    std::ostream& stream();

    /**
     * @brief Print a message to log
     * @param[in] msg Message to print to log
     * @returns Reference to this logger instance
     */
    Logger& print(const std::string& msg);

private:
    /// Pointer to implementatin details
    pimpl_ptr m_pimpl_;
};

/// Create STDOUT logger
Logger make_stdout_logger();
/// Create STDERR logger
Logger make_stderr_logger();

/**
 * @brief Create File Logger
 *
 * Produced logger will print contents to a specified file
 *
 * @param[in] fname Filename for specified logger
 */
Logger make_file_logger(std::string fname);

/**
 * @brief Create a Null logger
 *
 * Prints contents into the void.
 */
Logger make_null_logger();

/**
 * @brief Create stream logger
 *
 * Prints contents to a specified stream.
 *
 * @param[in] s Stream from which to generate logger.
 */
Logger make_stream_logger(std::ostream* s);

} // namespace parallelzone
