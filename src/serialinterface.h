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

// Type definitions
typedef uint16_t address;
typedef uint8_t byte;


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

    // Low-level port interface
    void set_DTR(bool value);
    void set_RTS(bool value);
    bool get_DSR();
    bool get_CTS();
    int read_device(unsigned char *data, size_t size);
    int write_device(const unsigned char *data, size_t size);

    // Bit-level I/O operations
    byte read_bit();
    void write_bit(bool bit);

    // Byte-level I/O operations
    byte read_byte();
    bool write_byte(byte value, bool verify = true);
    void read_next();

    // Generic I/O operations
    std::vector<byte> read_data(address location, size_t length);
    bool write_data(address location, const std::vector<byte> &data);

    // Command interface
    bool send_command(byte command, bool verify = true);
    void start_sequence();
    void end_command();

    // Auxiliary
    void nanodelay();
    bool query_address(address location);
private:
    int _sp;
};

#endif
