//
// Configuration
//

// Include guard
#ifndef _OPENLACROSSE_STATION_
#define _OPENLACROSSE_STATION_

// Standard library
#include <stdexcept>
#include <vector>
#include <ostream>

// Boost
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

// Local includes
#include "global.h"

// TODO's
// - Don't use an abstract superclass, but provide stubs which throw an
//   UnsupportedException
// - UnsupportedException should be thrown when trying to access unsupported
//   values, such as rain or wind, due to the format string requesting them.


//
// Module definitions
//

class ProtocolException : std::runtime_error
{
public:
    ProtocolException(const std::string& message)
        : std::runtime_error(message) { };
};

class Station
{
public:
    // Subclasses
    struct SensorRecord
    {
        // FIXME: initializer list doesn't work due to ambiguity
        SensorRecord(double temperature, unsigned int humidity)
            : temperature(temperature), humidity(humidity) { }
        double temperature;
        unsigned int humidity;
    };
    struct HistoryRecord
    {
        ptime datetime;
        SensorRecord internal;
        std::vector<SensorRecord> outdoor;
    };

    // Construction and destruction
    virtual ~Station() { }

    // Station properties
    virtual unsigned int external_sensors() = 0;

    // History management
    virtual HistoryRecord history(int record_no) = 0;
    virtual int history_count() = 0;
    virtual ptime history_modtime() = 0;
    virtual HistoryRecord history_first() = 0;
    virtual HistoryRecord history_last() = 0;
    virtual bool history_reset() = 0;
};

// Operators
std::ostream & operator<<(std::ostream &os, const Station::HistoryRecord &hr);

#endif
