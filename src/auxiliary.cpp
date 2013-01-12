//
// Configuration
//


// Header
#include "auxiliary.h"

// Standard library
#include <sstream>

// Boost
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

// Null stream
std::ostream cnull(0);


//
// Construction and destruction
//
//
Logger::Logger() : _buf(std::cout.rdbuf()) {
    std::cout.flush();
    std::cout.rdbuf(&_buf);    
}


//
// Logging
// 

std::ostream& Logger::log(LogLevel level)
{
    if (level <= THRESHOLD) {
        char last_char = _buf.last_char();
        if (last_char == '\r' || last_char == '\n')
            std::cout << timestamp() << "  " << prefix(level) << "\t";
        return std::cout;
    } else {
        return cnull;
    }
}


//
// Auxiliary
//

std::string Logger::timestamp()
{
    ptime now = microsec_clock::universal_time();
    return to_iso_extended_string(now);
}

std::string Logger::prefix(LogLevel level)
{
    switch (level) {
        case fatal:
            return "FATAL";
        case error:
            return "ERROR";
        case warning:
            return "WARNING";
        case info:
            return "INFO";
        case debug:
            return "DEBUG";
        case trace:
            return "TRACE";
        default:
            int base;
            if (level < fatal)
                base = fatal;
            else
                base = trace;
            int diff = level - base;

            std::stringstream ss;
            ss << prefix((LogLevel) base) << std::showpos << diff;

            return ss.str();

    }
}

//
// Syntax sugar
//

std::ostream& clog(LogLevel level)
{
    return logger.log(level);
}
