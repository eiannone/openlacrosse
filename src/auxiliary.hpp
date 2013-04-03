//
// Configuration
//

// Include guard
#ifndef _OPENLACROSSE_AUXILIARY_
#define _OPENLACROSSE_AUXILIARY_

// Standard library
#ifdef USE_PCE
#include "std_includes.hpp"
#else
#include <iostream>
#endif

// Boost
#ifdef USE_PCE
#include "boost_includes.hpp"
#else
#include <boost/integer.hpp>
#endif


//
// Module definitions
//

// Log levels
enum LogLevel {
    fatal = -3,
    error,
    warning,
    info = 0,
    debug,
    trace
};

// Streambuffer
class keepbuf : public std::streambuf {
public:
    keepbuf(std::streambuf* buf) : _buf(buf), _last_char('\n') {
        // traits_type::eof would be better, but this fits our use case
        // no buffering, overflow on every char
        setp(0, 0);
    }
    char last_char() const { return _last_char; }

    virtual int_type overflow(int_type c) {
        _buf->sputc(c);
        _last_char = c;
        return c;
    }
private:
    std::streambuf* _buf;
    char _last_char;
};

// Null stream
extern std::ostream cnull;

// Logger
class Logger {
public:
    Logger();

    // Configuration
    struct
    {
        LogLevel threshold;
        bool prefix_timestamp;
        bool prefix_level;
    } settings;

    // Logging
    std::ostream& log(LogLevel level);

private:
    // Auxiliary
    static std::string timestamp();
    static std::string prefix(LogLevel level);
    
    // Replacement stream buffer
    keepbuf _buf;
};
extern Logger logger;

// Syntax sugar
std::ostream& clog(LogLevel level);

#endif
