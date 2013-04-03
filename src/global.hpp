//
// Configuration
//

// Include guard
#ifndef _OPENLACROSSE_GLOBAL_
#define _OPENLACROSSE_GLOBAL_

// Boost
#ifdef USE_PCE
#include "boost_includes.hpp"
#else
#include <boost/integer.hpp>
#endif

// Type definitions
typedef uint8_t byte;
typedef uint16_t address;

#endif
