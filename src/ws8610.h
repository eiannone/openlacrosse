//
// Configuration
//

// Include guard
#ifndef _OPENLACROSSE_WS8610_
#define _OPENLACROSSE_WS8610_

// Standard library
#include <vector>
#include <stdexcept>

// Boost
#include <boost/integer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

// Local includes
#include "serialinterface.h"


//
// Module definitions
//

class ProtocolException : std::runtime_error
{
public:
    ProtocolException(const std::string& message)
        : std::runtime_error(message) { };
};

class WS8610
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
    WS8610(const std::string& portname);

    // Station properties
    uint8_t external_sensors();

    // History management
    HistoryRecord history(int record_no);
    int history_count();
    ptime history_modtime();
    HistoryRecord history_first();
    HistoryRecord history_last();
    bool history_reset();

private:
    // Auxiliary
    std::vector<byte> read_safe(address location, size_t length);
    std::vector<byte> memory(address location, size_t length);
    static ptime parse_datetime(const std::vector<byte> &data);
    static double parse_temperature(const std::vector<byte> &data, int sensor);
    static unsigned int parse_humidity(const std::vector<byte> &data, int sensor);

    // Communication interface
    SerialInterface _iface;

    // Station characteristics
    unsigned int _external_sensors;
    unsigned int _record_size;
    unsigned int _max_records;
};

// Operators
std::ostream & operator<<(std::ostream &os, const WS8610::HistoryRecord &hr);

#endif
