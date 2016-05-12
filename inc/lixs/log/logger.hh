#ifndef __LIXS_LOG_LOGGER_HH__
#define __LIXS_LOG_LOGGER_HH__

#include <stdexcept>
#include <string>
#include <utility>


#define LOGGER_MAX_LEVEL_TRACE 5
#define LOGGER_MAX_LEVEL_DEBUG 4
#define LOGGER_MAX_LEVEL_INFO  3
#define LOGGER_MAX_LEVEL_WARN  2
#define LOGGER_MAX_LEVEL_ERROR 1
#define LOGGER_MAX_LEVEL_OFF   0

#ifndef LOGGER_MAX_LEVEL
#define LOGGER_MAX_LEVEL LOGGER_MAX_LEVEL_INFO
#endif


namespace lixs {
namespace log {

enum level {
    TRACE = 5 ,
    DEBUG = 4 ,
    INFO  = 3 ,
    WARN  = 2 ,
    ERROR = 1 ,
    OFF   = 0 ,
};

class logger {
public:
    logger(level logger_level)
        : logger_level(logger_level)
    {
        fp = stdout;
    };

    logger(level logger_level, const std::string& path)
        : logger_level(logger_level)
    {
        fp = std::fopen(path.c_str(), "w");
        if (fp == NULL) {
            throw std::runtime_error("Failed to open log file!");
        }
    };

    ~logger()
    {
        if (fp != stdout) {
            std::fclose(fp);
        }
    };

public:
    template < typename... ARGS >
    void logf(level l, const std::string& format, ARGS&&... args);

private:
    level logger_level;
    std::FILE* fp;
};


template < typename... ARGS >
void logger::logf(level l, const std::string& format, ARGS&&... args)
{
    if ( logger_level >= l ) {
        switch (l) {
            case TRACE:
                std::fputs("TRACE ", fp);
                break;

            case DEBUG:
                std::fputs("DEBUG ", fp);
                break;

            case INFO:
                std::fputs("INFO  ", fp);
                break;

            case WARN:
                std::fputs("WARN  ", fp);
                break;

            case ERROR:
                std::fputs("ERROR ", fp);
                break;

            case OFF:
                return;
        }

        std::fprintf(fp, format.c_str(), std::forward<ARGS>(args)...);

        std::fputc('\n', fp);

        std::fflush(fp);
    }
}


template < level LEVEL >
class LOG {
public:
    template < typename... ARGS >
    static void logf(logger& log, const std::string& format, ARGS&&... args);
};

template < >
class LOG<level::ERROR> {
public:
    template < typename... ARGS >
    static inline void logf(logger& log, const std::string& format, ARGS&&... args) {
#if LOGGER_MAX_LEVEL >= LOGGER_MAX_LEVEL_ERROR
        log.logf(level::ERROR, format, std::forward<ARGS>(args)...);
#endif
    }
};

template < >
class LOG<level::WARN> {
public:
    template < typename... ARGS >
    static inline void logf(logger& log, const std::string& format, ARGS&&... args) {
#if LOGGER_MAX_LEVEL >= LOGGER_MAX_LEVEL_WARN
        log.logf(level::WARN, format, std::forward<ARGS>(args)...);
#endif
    }
};

template < >
class LOG<level::INFO> {
public:
    template < typename... ARGS >
    static inline void logf(logger& log, const std::string& format, ARGS&&... args) {
#if LOGGER_MAX_LEVEL >= LOGGER_MAX_LEVEL_INFO
        log.logf(level::INFO, format, std::forward<ARGS>(args)...);
#endif
    }
};

template < >
class LOG<level::DEBUG> {
public:
    template < typename... ARGS >
    static inline void logf(logger& log, const std::string& format, ARGS&&... args) {
#if LOGGER_MAX_LEVEL >= LOGGER_MAX_LEVEL_DEBUG
        log.logf(level::DEBUG, format, std::forward<ARGS>(args)...);
#endif
    }
};

template < >
class LOG<level::TRACE> {
public:
    template < typename... ARGS >
    static inline void logf(logger& log, const std::string& format, ARGS&&... args) {
#if LOGGER_MAX_LEVEL >= LOGGER_MAX_LEVEL_TRACE
        log.logf(level::TRACE, format, std::forward<ARGS>(args)...);
#endif
    }
};

} /* namespace log */
} /* namespace lixs */

#endif /* __LIXS_LOG_LOGGER_HH__ */

