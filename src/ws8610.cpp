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

    _iface.set_DTR(0);
    _iface.set_RTS(0);

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
        _iface.set_RTS(1);
        _iface.set_DTR(1);
    } else {
        throw ProtocolException("Connection timeout (did not clear DSR)");
    }

    _iface.write_device(buffer, 448);

    // Configure communication properties dependant on
    // the amount of external sensors
    _external_sensors = GetExternalSensors();
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

std::vector<uint8_t> WS8610::read_safe(short address, int uint8_ts_to_read)
{
    //unsigned char readdata2[32768];
    clog(trace) << "read_safe" << std::endl;
    std::vector<uint8_t> readdata, readdata2;
    int j;
    for (j = 0; j < MAX_READ_RETRIES; j++)
    {
        _iface.start_sequence();
        readdata = _iface.read_data(address, uint8_ts_to_read);
        _iface.start_sequence();
        readdata2 = _iface.read_data(address, uint8_ts_to_read);
        if (readdata.size() == 0 || readdata != readdata2)
        {
            clog(debug) << "read_safe - two readings not identical" << std::endl;
            continue;
        }
        //check if only 0's for reading memory range greater then 10 uint8_ts
        clog(debug) << "read_safe - two readings identical" << std::endl;
        int i = 0;
        if (uint8_ts_to_read > 10)
            for (; readdata[i] == 0 && i < uint8_ts_to_read; i++)
            { }

        if (i != uint8_ts_to_read)
            break;
        clog(debug) << "read_safe - only zeros" << std::endl;
    }
    // If we have tried MAX_READ_RETRIES times to read we expect not to have valid data
    if (j == MAX_READ_RETRIES)
        throw ProtocolException("Safe read failed");

    return readdata;
}

std::vector<uint8_t> WS8610::DumpMemory(short start_addr, short num_uint8_ts)
{
    short end_addr = start_addr + num_uint8_ts - 1;
    if (start_addr < 0 || end_addr > 0x7FFF)
    {
        //throw "Invalid address range: " + hex(start_addr) + " - " + hex(end_addr));
        throw ProtocolException("Invalid address range");
    }
    return read_safe(start_addr, num_uint8_ts);
}

WS8610::HistoryRecord WS8610::read_history_record(int record_no)
{
    while (record_no >= _max_records)
        record_no -= _max_records;

    short addr = (short)(HISTORY_BASE_ADDR + record_no * _record_size);
    std::vector<uint8_t> record = read_safe(addr, _record_size);
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

    int min = (record[0] >> 4) * 10 + (record[0] & 0x0F);
    int hour = (record[1] >> 4) * 10 + (record[1] & 0x0F);
    int mday = (record[2] >> 4) * 10 + (record[2] & 0x0F);
    int mon = (record[3] >> 4) * 10 + (record[3] & 0x0F);
    int year = (record[4] >> 4) * 10 + (record[4] & 0x0F) + 2000;
    ptime DateTime(date(year, mon, mday), hours(hour) + minutes(min));

    auto Temp = std::vector<double>(_external_sensors + 1);
    Temp[0] = ((record[6] & 0x0F) * 10 + (record[5] >> 4) + (record[5] & 0x0F) / 10.0) - 30.0;
    for (auto s = 1; s <= _external_sensors; s++)
        Temp[s] = temperature_outdoor(record, s);

    auto Hum = std::vector<int>(_external_sensors + 1);
    Hum[0] = (record[8] >> 4) * 10 + (record[8] & 0xF);
    for (auto s = 1; s <= _external_sensors; s++)
        Hum[s] = humidity_outdoot(record, s);


    HistoryRecord hr{DateTime, Temp, Hum};

    return hr;
}

uint8_t WS8610::GetExternalSensors()
{
    std::vector<uint8_t> data = read_safe(0x0C, 1);
    if (data.size() == 0)
        throw ProtocolException("Invalid external sensor count");
    return data[0] & 0x0F;
}

ptime WS8610::GetLastRecordDateTime()
{
    std::vector<uint8_t> dt = read_safe(0x0000, 6);
    if (dt.size() != 6)
        throw ProtocolException("Cannot obtain last recording date/time");

    int min = ((dt[0] >> 4) * 10) + (dt[0] & 0x0F);
    int hour = ((dt[1] >> 4) * 10) + (dt[1] & 0x0F);
    int day = (dt[2] >> 4) + ((dt[3] & 0x0F) * 10);
    int month = (dt[3] >> 4) + ((dt[4] & 0x0F) * 10);
    int year = 2000 + (dt[4] >> 4) + ((dt[5] & 0xF) * 10);

    return ptime(date(year, month, day), hours(hour) + minutes(min));
}

WS8610::HistoryRecord WS8610::GetFirstHistoryRecord()
{
    return read_history_record(0);
}

WS8610::HistoryRecord WS8610::GetLastHistoryRecord()
{
    auto first_rec = read_history_record(0);
    ptime dt_last = GetLastRecordDateTime();
    time_duration difference = dt_last - first_rec.Datetime;
    int tot_records = 1 + (60*difference.hours() + difference.minutes()) / 5;

    clog(debug) << "Tot history: " << tot_records << " records" << std::endl;
    clog(debug) << "Date first: " << first_rec.Datetime << std::endl;
    clog(debug) << "Date last: " << dt_last << std::endl;

    // Try to see if record (n+1) is valid
    auto check = read_safe((short)(HISTORY_BASE_ADDR + (tot_records * _record_size)), 1);

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
    return read_history_record(tot_records - 1);
}



/// <summary>
/// Get number of history records stored in memory
/// </summary>
/// <returns>Number of history records stored in memory</returns>
int WS8610::GetStoredHistoryCount()
{
    auto b = read_safe(0x0009, 2);
    return (b[0] >> 4) * 1000 + (b[1] & 0x0F) * 100 + (b[0] >> 4) * 10 + (b[0] & 0x0F);
}

/// <summary>
/// Reset 'mem' indicator to 0000. Next history data will be stored at position 0.
/// </summary>
/// <returns>true if success</returns>
bool WS8610::ResetStoredHistoryCount()
{
    return _iface.WriteMemory(0x0009, std::vector<uint8_t>{0x00, 0x00});
}

double WS8610::temperature_outdoor(std::vector<uint8_t> rec, int num)
{
    switch (num)
    {
        case 2:
            return ((rec[11] & 0x0F) * 10 + (rec[10] >> 4) + (rec[10] & 0x0F) / 10.0) - 30.0;
        case 3:
            return ((rec[13] & 0x0F) + (rec[13] >> 4) * 10 + (rec[12] >> 4) / 10.0) - 30.0;
        default:
            return ((rec[7] & 0x0F) + (rec[7] >> 4) * 10 + (rec[6] >> 4) / 10.0) - 30.0;
    }
}

int WS8610::humidity_outdoot(std::vector<uint8_t> rec, int num)
{
    switch (num)
    {
        case 2:
            return (rec[11] >> 4) + (rec[12] & 0x0F) * 10;
        case 3:
            return (rec[14] >> 4) * 10 + (rec[14] & 0x0F);
        default:
            return (rec[9] >> 4) * 10 + (rec[9] & 0x0F);
    }
}
