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
    if (hr.external.size() > 0) {
        os << " (";
        for (size_t i = 0; i < hr.external.size(); i++) {
            os << hr.external[i].temperature << "°C " << hr.external[i].humidity << "%";
            if (i < hr.external.size()-1)
                os << ", ";
        }
        os << ")";
    }
        os << " at " << hr.datetime;
    return os;
}
