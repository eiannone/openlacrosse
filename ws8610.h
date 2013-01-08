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
const short HISTORY_BASE_ADDR = 0x0064;                        // Starting address where history data is stored
const short HISTORY_BUFFER_SIZE = 0x7FFF - HISTORY_BASE_ADDR;   // Size of history buffer, in bytes


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
        ptime Datetime;
        std::vector<double> Temp;
        std::vector<int> Hum;
    };

    // Construction and destruction
    WS8610(const std::string& portname);

    std::vector<uint8_t> read_safe(short address, int uint8_ts_to_read);
    std::vector<uint8_t> DumpMemory(short start_addr, short num_uint8_ts);
    HistoryRecord read_history_record(int record_no);
    uint8_t GetExternalSensors();
    ptime GetLastRecordDateTime();
    HistoryRecord GetFirstHistoryRecord();
    HistoryRecord GetLastHistoryRecord();
    int GetStoredHistoryCount();
    bool ResetStoredHistoryCount();
    double temperature_outdoor(std::vector<uint8_t> rec, int num);
    int humidity_outdoot(std::vector<uint8_t> rec, int num);

private:
    SerialInterface _iface;

    uint8_t _external_sensors;
    uint8_t _record_size;
    uint8_t _max_records;
};

#endif
