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

// Configurable values
#define INIT_WAIT 500
#define MAX_READ_RETRIES 20
const address HISTORY_START_LOCATION = 0x0064;
const address HISTORY_END_LOCATION = 0x7FFF;
const address HISTORY_BUFFER_SIZE = HISTORY_END_LOCATION - HISTORY_START_LOCATION;


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
    struct HistoryRecord
    {
        ptime datetime;
        unsigned int sensors;
        std::vector<double> temperature;
        std::vector<int> humidity;
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
    static int parse_humidity(const std::vector<byte> &data, int sensor);

    // Communication interface
    SerialInterface _iface;

    // Station characteristics
    uint8_t _external_sensors;
    uint8_t _record_size;
    uint8_t _max_records;
};

// Operators
std::ostream & operator<<(std::ostream &os, const WS8610::HistoryRecord &hr);

#endif
