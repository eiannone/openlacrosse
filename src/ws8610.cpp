//
// Configuration
//

// Header include
#include "ws8610.h"

// Platform
#include <unistd.h>

// Local includes
#include "auxiliary.h"

// Boost
using namespace boost::gregorian;


//
// Construction and destruction
//

WS8610::WS8610(const std::string& portname) : _iface(portname)
{
    // Send the magic initialization string
    unsigned char buffer[BUFFER_SIZE];
    for (int i = 0; i < 448; i++)
        buffer[i] = 'U';
    _iface.write_device(buffer, 448);

    _iface.set_DTR(false);
    _iface.set_RTS(false);

    int i = 0;
    do {
        usleep(10000);
        i++;
    } while (i < INIT_WAIT && !_iface.get_DSR());
    if (i == INIT_WAIT)
        throw ProtocolException("Connection timeout (did not set DSR)");

    i = 0;
    do {
        usleep(10000);
        i++;
    } while (i < INIT_WAIT && _iface.get_DSR());
    if (i != INIT_WAIT) {
        _iface.set_RTS(true);
        _iface.set_DTR(true);
    } else {
        throw ProtocolException("Connection timeout (did not clear DSR)");
    }

    _iface.write_device(buffer, 448);

    // Configure communication properties dependant on
    // the amount of external sensors
    _external_sensors = external_sensors();
    switch (_external_sensors) {
        case 1:
            _record_size = 10;
            break;
        case 2:
            _record_size = 13;
            break;
        case 3:
            _record_size = 15;
            break;
        default:
            throw ProtocolException("Unsupported amount of external sensors");
    }
    _max_records = HISTORY_BUFFER_SIZE / _record_size;
}

//
// Station properties
//

uint8_t WS8610::external_sensors()
{
    std::vector<byte> data = read_safe(0x0C, 1);
    if (data.size() == 0)
        throw ProtocolException("Invalid external sensor count");
    return data[0] & 0x0F;
}


//
// History management
//

WS8610::HistoryRecord WS8610::history(int record_no)
{
    while (record_no >= _max_records)
        record_no -= _max_records;

    short addr = (short)(HISTORY_START_LOCATION + record_no * _record_size);
    std::vector<byte> record = read_safe(addr, _record_size);
    if (record.size() != _record_size)
        throw ProtocolException("Invalid history data received");

    clog(debug) << _external_sensors
        << " additional sensor(s), record length is " << _record_size
        << ", " << "max record count is " << _max_records << std::endl;
    clog(debug) << "Reading record n. " << record_no << " at 0x"
        << std::hex << addr << ":";
    for (size_t i = 0; i < _record_size; i++)
        clog(debug) << " " << std::hex << record[i];
    clog(debug) << std::endl;

    ptime datetime = parse_datetime(record);

    auto temperature = std::vector<double>(_external_sensors + 1);
    for (auto s = 0; s <= _external_sensors; s++)
        temperature[s] = parse_temperature(record, s);

    auto humidity = std::vector<int>(_external_sensors + 1);
    for (auto s = 0; s <= _external_sensors; s++)
        humidity[s] = parse_humidity(record, s);

    HistoryRecord hr{datetime, temperature, humidity};

    return hr;
}

/// <summary>
/// Get number of history records stored in memory
/// </summary>
/// <returns>Number of history records stored in memory</returns>
int WS8610::history_count()
{
    auto b = read_safe(0x0009, 2);
    return (b[0] >> 4) * 1000 + (b[1] & 0x0F) * 100 + (b[0] >> 4) * 10 + (b[0] & 0x0F);
}

ptime WS8610::history_modtime()
{
    std::vector<byte> dt = read_safe(0x0000, 6);
    if (dt.size() != 6)
        throw ProtocolException("Cannot obtain last recording date/time");

    int min = ((dt[0] >> 4) * 10) + (dt[0] & 0x0F);
    int hour = ((dt[1] >> 4) * 10) + (dt[1] & 0x0F);
    int day = (dt[2] >> 4) + ((dt[3] & 0x0F) * 10);
    int month = (dt[3] >> 4) + ((dt[4] & 0x0F) * 10);
    int year = 2000 + (dt[4] >> 4) + ((dt[5] & 0xF) * 10);

    return ptime(date(year, month, day), hours(hour) + minutes(min));
}

WS8610::HistoryRecord WS8610::history_first()
{
    return history(0);
}

WS8610::HistoryRecord WS8610::history_last()
{
    auto first_rec = history(0);
    ptime dt_last = history_modtime();
    time_duration difference = dt_last - first_rec.datetime;
    int tot_records = 1 + (60*difference.hours() + difference.minutes()) / 5;

    clog(debug) << "Tot history: " << tot_records << " records" << std::endl;
    clog(debug) << "Date first: " << first_rec.datetime << std::endl;
    clog(debug) << "Date last: " << dt_last << std::endl;

    // Try to see if record (n+1) is valid
    auto check = read_safe((short)(HISTORY_START_LOCATION + (tot_records * _record_size)), 1);

    clog(debug) << "Next record start with " << std::hex << check[0] << std::endl;
    // If valid then read one record ahead
    if (check[0] != 0xFF)
    {
        clog(debug) << "Seems valid, skipping to it" << std::endl;
        tot_records++;
    }
    else
    {
        clog(debug) << "Seems invalid, stick with current record" << std::endl;
    }
    return history(tot_records - 1);
}

/// <summary>
/// Reset 'mem' indicator to 0000. Next history data will be stored at position 0.
/// </summary>
/// <returns>true if success</returns>
bool WS8610::history_reset()
{
    return _iface.write_data(0x0009, std::vector<byte>{0x00, 0x00});
}


//
// Auxiliary
//

std::vector<byte> WS8610::read_safe(address location, size_t length)
{
    clog(trace) << "read_safe" << std::endl;
    std::vector<byte> readdata, readdata2;
    int j;
    for (j = 0; j < MAX_READ_RETRIES; j++)
    {
        _iface.start_sequence();
        readdata = _iface.read_data(location, length);
        _iface.start_sequence();
        readdata2 = _iface.read_data(location, length);
        if (readdata.size() == 0 || readdata != readdata2)
        {
            clog(debug) << "read_safe - two readings not identical" << std::endl;
            continue;
        }
        //check if only 0's for reading memory range greater then 10 bytes
        clog(debug) << "read_safe - two readings identical" << std::endl;
        int i = 0;
        if (length > 10)
            for (; readdata[i] == 0 && i < length; i++)
            { }

        if (i != length)
            break;
        clog(debug) << "read_safe - only zeros" << std::endl;
    }
    // If we have tried MAX_READ_RETRIES times to read we expect not to have valid data
    if (j == MAX_READ_RETRIES)
        throw ProtocolException("Safe read failed");

    return readdata;
}

std::vector<byte> WS8610::memory(address location, size_t length)
{
    address end_location = location + length - 1;
    if (location < 0 || end_location > HISTORY_END_LOCATION)
    {
        //throw "Invalid address range: " + hex(address) + " - " + hex(end_addr));
        throw ProtocolException("Invalid address range");
    }
    return read_safe(location, length);
}

ptime WS8610::parse_datetime(const std::vector<byte> &data)
{
    int min = (data[0] >> 4) * 10 + (data[0] & 0x0F);
    int hour = (data[1] >> 4) * 10 + (data[1] & 0x0F);
    int mday = (data[2] >> 4) * 10 + (data[2] & 0x0F);
    int mon = (data[3] >> 4) * 10 + (data[3] & 0x0F);
    int year = (data[4] >> 4) * 10 + (data[4] & 0x0F) + 2000;

    return ptime(date(year, mon, mday), hours(hour) + minutes(min));
}

double WS8610::parse_temperature(const std::vector<byte> &data, int sensor)
{
    switch (sensor)
    {
        case 0:
            return ((data[6] & 0x0F) * 10 + (data[5] >> 4)
                + (data[5] & 0x0F) / 10.0) - 30.0;
        case 1:
            return ((data[7] & 0x0F) + (data[7] >> 4) * 10
                + (data[6] >> 4) / 10.0) - 30.0;
        case 2:
            return ((data[11] & 0x0F) * 10 + (data[10] >> 4)
                + (data[10] & 0x0F) / 10.0) - 30.0;
        case 3:
            return ((data[13] & 0x0F) + (data[13] >> 4) * 10
                + (data[12] >> 4) / 10.0) - 30.0;
        default:
            throw ProtocolException("Invalid sensor");
    }
}

int WS8610::parse_humidity(const std::vector<byte> &data, int sensor)
{
    switch (sensor)
    {
        case 0:
            return (data[8] >> 4) * 10 + (data[8] & 0xF);
        case 1:
            return (data[9] >> 4) * 10 + (data[9] & 0x0F);
        case 2:
            return (data[11] >> 4) + (data[12] & 0x0F) * 10;
        case 3:
            return (data[14] >> 4) * 10 + (data[14] & 0x0F);
        default:
            throw ProtocolException("Invalid sensor");
    }
}
