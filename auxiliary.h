//
// Configuration
//

// Include guard
#ifndef _OPENLACROSSE_AUXILIARY_
#define _OPENLACROSSE_AUXILIARY_

// Standard library
#include <iostream>

// Boost
#include <boost/integer.hpp>

// Configurable values
#define THRESHOLD 0


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

// Logging
extern std::ostream cnull;
std::ostream& clog(uint8_t level);

#endif
