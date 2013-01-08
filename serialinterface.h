//
// Configuration
//

// Include guard
#ifndef _OPENLACROSSE_SERIALINTERFACE_
#define _OPENLACROSSE_SERIALINTERFACE_

// Standard library
#include <string>
#include <vector>
#include <stdexcept>

// Boost
#include <boost/integer.hpp>

// Configurable values
#define BAUDRATE B300
#define BUFFER_SIZE 16384


//
// Module definitions
//

class HardwareException : std::runtime_error
{
public:
    HardwareException(const std::string& message)
        : std::runtime_error(message) { };
};

class SerialInterface
{
public:
    // Construction and destruction
    SerialInterface(const std::string& portname);
    ~SerialInterface();

    void set_DTR(bool val);
    void set_RTS(bool val);
    bool get_DSR();
    bool get_CTS();
    int read_device(unsigned char *buffer, int size);
    int write_device(unsigned char *buffer, int size);
    void nanodelay();

    uint8_t read_bit();
    void write_bit(bool bit);

    uint8_t read_uint8_t();
    bool send_uint8_t(uint8_t b, bool checkvalue = true);
    void read_next();

    bool send_command(uint8_t cmd, bool checkvalue = true);
    void start_sequence();
    void end_command();

    bool query_address(short address);
    std::vector<uint8_t> read_data(short address, int uint8_ts_to_read);
    bool WriteMemory(short start_addr, std::vector<uint8_t> uint8_ts_to_write);
private:
    int _sp;
};

#endif
