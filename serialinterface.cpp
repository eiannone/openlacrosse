//
// Configuration
//

// Header
#include "serialinterface.h"

// Standard library
#include <string.h>

// Platform
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/file.h>

// Local includes
#include "auxiliary.h"


//
// Construction and destruction
//

SerialInterface::SerialInterface(const std::string& portname)
{
    // Open the port
    clog(info) << "open_weatherstation" << std::endl;
    if ((_sp = open(portname.c_str(), O_RDWR | O_NOCTTY)) < 0)
        throw HardwareException("Unable to open serial device");
    if ( flock(_sp, LOCK_EX) < 0 )
        throw HardwareException("Serial device is locked by other program");

    // We want full control of what is set by simply resetting entire adtio
    // struct    
    struct termios adtio;
    memset(&adtio, 0, sizeof(adtio));

    // Serial control options
    adtio.c_cflag &= ~PARENB;      // No parity
    adtio.c_cflag &= ~CSTOPB;      // One stop bit
    adtio.c_cflag &= ~CSIZE;       // Character size mask
    adtio.c_cflag |= CS8;          // Character size 8 bits
    adtio.c_cflag |= CREAD;        // Enable Receiver
    //adtio.c_cflag &= ~CREAD;        // Disable Receiver
    adtio.c_cflag &= ~HUPCL;       // No "hangup"
    adtio.c_cflag &= ~CRTSCTS;     // No flowcontrol
    adtio.c_cflag |= CLOCAL;       // Ignore modem control lines

    // Baudrate, for newer systems
    cfsetispeed(&adtio, BAUDRATE);
    cfsetospeed(&adtio, BAUDRATE);

    // Local options
    //   Raw input = clear ICANON, ECHO, ECHOE, and ISIG
    //   Disable misc other local features = clear FLUSHO, NOFLSH, TOSTOP, PENDIN, and IEXTEN
    // So we actually clear all flags in adtio.c_lflag
    adtio.c_lflag = 0;

    // Input options
    //   Disable parity check = clear INPCK, PARMRK, and ISTRIP
    //   Disable software flow control = clear IXON, IXOFF, and IXANY
    //   Disable any translation of CR and LF = clear INLCR, IGNCR, and ICRNL
    //   Ignore break condition on input = set IGNBRK
    //   Ignore parity errors just in case = set IGNPAR;
    // So we can clear all flags except IGNBRK and IGNPAR
    adtio.c_iflag = IGNBRK|IGNPAR;

    // Output options
    // Raw output should disable all other output options
    adtio.c_oflag &= ~OPOST;

    // Time-out options
    adtio.c_cc[VTIME] = 10;     // timer 1s
    adtio.c_cc[VMIN] = 0;       // blocking read until 1 char

    if (tcsetattr(_sp, TCSANOW, &adtio) < 0)
        throw HardwareException("Unable to initialize serial device");
    tcflush(_sp, TCIOFLUSH);
}

SerialInterface::~SerialInterface()
{
    tcflush(_sp, TCIOFLUSH);
    close(_sp);
}

void SerialInterface::set_DTR(bool val)
{
    // TODO: use TIOCMBIC and TIOCMBIS instead of TIOCMGET and TIOCMSET
    int portstatus;
    ioctl(_sp, TIOCMGET, &portstatus);   // get current port status
    if (val)
    {
        clog(trace) << "Set DTR" << std::endl;
        portstatus |= TIOCM_DTR;
    }
    else
    {
        clog(trace) << "Clear DTR" << std::endl;
        portstatus &= ~TIOCM_DTR;
    }
    ioctl(_sp, TIOCMSET, &portstatus);   // set current port status

    /*if (val)
      ioctl(_sp, TIOCMBIS, TIOCM_DTR);
    else
      ioctl(_sp, TIOCMBIC, TIOCM_DTR);*/
}

void SerialInterface::set_RTS(bool val)
{
    //TODO: use TIOCMBIC and TIOCMBIS instead of TIOCMGET and TIOCMSET
    int portstatus;
    ioctl(_sp, TIOCMGET, &portstatus);   // get current port status
    if (val)
    {
        clog(trace) << "Set RTS" << std::endl;
        portstatus |= TIOCM_RTS;
    }
    else
    {
        clog(trace) << "Clear RTS" << std::endl;
        portstatus &= ~TIOCM_RTS;
    }
    ioctl(_sp, TIOCMSET, &portstatus);   // set current port status

    /*if (val)
      ioctl(_sp, TIOCMBIS, TIOCM_RTS);
    else
      ioctl(_sp, TIOCMBIC, TIOCM_RTS);
    */

}

bool SerialInterface::get_DSR()
{
    int portstatus;
    ioctl(_sp, TIOCMGET, &portstatus);   // get current port status

    if (portstatus & TIOCM_DSR)
    {
        clog(trace) << "Got DSR = 1" << std::endl;
        return true;
    }
    else
    {
        clog(trace) << "Got DSR = 0" << std::endl;
        return false;
    }
}

bool SerialInterface::get_CTS()
{
    int portstatus;
    ioctl(_sp, TIOCMGET, &portstatus);   // get current port status

    if (portstatus & TIOCM_CTS)
    {
        clog(trace) << "Got CTS = 1" << std::endl;
        return true;
    }
    else
    {
        clog(trace) << "Got CTS = 0" << std::endl;
        return false;
    }
}

int SerialInterface::read_device(unsigned char *buffer, int size)
{
    int ret;

    for (;;) {
        ret = read(_sp, buffer, size);
        if (ret == 0 && errno == EINTR)
            continue;
        return ret;
    }
}

int SerialInterface::write_device(unsigned char *buffer, int size)
{
    int ret = write(_sp, buffer, size);
    return ret;
}

void SerialInterface::nanodelay()
{
    usleep(4);
}

uint8_t SerialInterface::read_bit()
{
    clog(trace) << "Read bit ..." << std::endl;
    set_DTR(false);
    nanodelay();
    bool status = get_CTS();
    nanodelay();
    set_DTR(true);
    nanodelay();
    clog(trace) << "Bit = " << (status ? "0" : "1") << std::endl;

    return (uint8_t)(status ? 0 : 1);
}

void SerialInterface::write_bit(bool bit)
{
    clog(trace) << "Write bit " << (bit ? "1" : "0") << std::endl;
    set_RTS(!bit);
    nanodelay();
    set_DTR(false);
    nanodelay();
    set_DTR(true);
}

uint8_t SerialInterface::read_uint8_t()
{
    clog(trace) << "Read byte ..." << std::endl;
    uint8_t b = 0;
    for (size_t i = 0; i < 8; i++)
    {
        b *= 2;
        b += read_bit();
    }
    clog(trace) << "byte = 0x" << std::hex << b << std::endl;
    return b;
}

bool SerialInterface::send_uint8_t(uint8_t b, bool checkvalue)
{
    clog(trace) << "Send byte 0x" << std::hex << b << std::endl;

    for (size_t i = 0; i < 8; i++)
    {
        write_bit((b & 0x80) > 0);
        b <<= 1;
    }
    set_RTS(false);
    nanodelay();
    bool status = true;
    if (checkvalue)
    {
        status = get_CTS();
        //TODO: checking value of status, error routine
        nanodelay();
        set_DTR(false);
        nanodelay();
        set_DTR(true);
        nanodelay();
    }
    return status;
}

void SerialInterface::read_next()
{
    clog(debug) << "read_next_byte_seq" << std::endl;
    set_RTS(true);
    nanodelay();
    set_DTR(false);
    nanodelay();
    set_DTR(true);
    nanodelay();
    set_RTS(false);
    nanodelay();
}

bool SerialInterface::send_command(uint8_t cmd, bool checkvalue)
{
    set_DTR(false);
    nanodelay();
    set_RTS(false);
    nanodelay();
    set_RTS(true);
    nanodelay();
    set_DTR(true);
    nanodelay();
    set_RTS(false);
    nanodelay();

    return send_uint8_t(cmd, checkvalue);
}

void SerialInterface::start_sequence()
{
    clog(trace) << "start_sequence" << std::endl;
    set_RTS(false);
    nanodelay();
    set_DTR(false);
    nanodelay();
}

void SerialInterface::end_command()
{
    clog(trace) << "end_command" << std::endl;
    set_RTS(true);
    nanodelay();
    set_DTR(false);
    nanodelay();
    set_RTS(false);
    nanodelay();
}

bool SerialInterface::query_address(short address)
{
    return send_command(0xA0) && send_uint8_t((uint8_t)(address / 256)) && send_uint8_t((uint8_t)(address % 256));
}

std::vector<uint8_t> SerialInterface::read_data(short address, int uint8_ts_to_read)
{
    if (!query_address(address) || !send_command(0xA1))
        return std::vector<uint8_t>();

    std::vector<uint8_t> readdata(uint8_ts_to_read);
    readdata[0] = read_uint8_t();
    for (size_t i = 1; i < uint8_ts_to_read; i++)
    {
        read_next();
        readdata[i] = read_uint8_t();
    }
    end_command();

    return readdata;
}

bool SerialInterface::WriteMemory(short start_addr, std::vector<uint8_t> bytes_to_write)
{
    start_sequence();
    if (!query_address(start_addr))
        return false;
    for (size_t i = 0; i < bytes_to_write.size(); i++)
        if (!send_uint8_t(bytes_to_write[i]))
            return false;
    end_command();

    start_sequence();
    for (size_t i = 0; i < 3; i++) 
        send_command(0xA0, false);

    set_DTR(false);
    nanodelay();
    bool result = get_CTS();
    set_DTR(true);
    nanodelay();

    return result;
}