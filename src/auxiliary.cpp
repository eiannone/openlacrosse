//
// Configuration
//


// Header
#include "auxiliary.h"


//
// Logging
//

std::ostream cnull(0);

std::ostream& clog(uint8_t level)
{
    if (level <= THRESHOLD)
        return std::cout;
    else
        return cnull;
}
