//
// Configuration
//

// Header include
#include "station.hpp"


//
// Operators
//

std::ostream & operator<<(std::ostream &os, const Station::HistoryRecord &hr)
{
    os << hr.internal.temperature << "°C " << hr.internal.humidity << "%";
    if (hr.external.size() > 0) {
        os << " (";
        for (size_t i = 0; i < hr.external.size(); i++) {
            if (hr.external[i].temperature)
                os << *hr.external[i].temperature;
            else
                os << "-";
            os << "°C ";

            if (hr.external[i].humidity)
                os << *hr.external[i].humidity;
            else
                os << "-";
            os << "%";
        }
        os << ")";
    }
        os << " at " << hr.datetime;
    return os;
}
