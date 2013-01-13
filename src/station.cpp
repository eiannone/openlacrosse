//
// Configuration
//

// Header include
#include "station.h"


//
// Operators
//

std::ostream & operator<<(std::ostream &os, const Station::HistoryRecord &hr)
{
    os << hr.internal.temperature << "°C " << hr.internal.humidity << "%";
    if (hr.outdoor.size() > 0) {
        os << " (";
        for (size_t i = 0; i < hr.outdoor.size(); i++) {
            os << hr.outdoor[i].temperature << "°C " << hr.outdoor[i].humidity << "%";
            if (i < hr.outdoor.size()-1)
                os << ", ";
        }
        os << ")";
    }
        os << " at " << hr.datetime;
    return os;
}
