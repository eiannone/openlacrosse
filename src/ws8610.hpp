//
// Configuration
//

// Include guard
#ifndef _OPENLACROSSE_WS8610_
#define _OPENLACROSSE_WS8610_

// Standard library
#include <vector>

// Boost
#include <boost/optional.hpp>

// Local includes
#include "global.hpp"
#include "station.hpp"
#include "serialinterface.hpp"

// TODO: min/max, dewpoint


//
// Module definitions
//

class WS8610 : public Station
{
public:
    // Construction and destruction
    WS8610(const std::string& portname);

    // Station properties
    unsigned int external_sensors();

    // History management
    HistoryRecord history(unsigned int record_no);
    int history_count();
    time_t history_modtime();
    HistoryRecord history_first();
    HistoryRecord history_last();
    bool history_reset();

private:
    // Auxiliary
    std::vector<byte> read_safe(address location, size_t length);
    std::vector<byte> memory(address location, size_t length);
    static time_t parse_datetime(const std::vector<byte> &data);
    static boost::optional<double> parse_temperature(const std::vector<byte> &data, int sensor);
    static boost::optional<unsigned int> parse_humidity(const std::vector<byte> &data, int sensor);

    // Communication interface
    SerialInterface _iface;

    // Station characteristics
    unsigned int _external_sensors;
    unsigned int _record_size;
    unsigned int _max_records;
};

#endif
