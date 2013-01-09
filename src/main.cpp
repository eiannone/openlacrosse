//
// Configuration
//

// Standard library
#include <iostream>

// Local includes
#include "ws8610.h"


//
// Main
//

int main()
{
    try {
        WS8610 station("/dev/ttyS0");

        auto record = station.GetLastHistoryRecord();
        std::cout << "Date/time last record: " << record.Datetime << std::endl;
        std::cout << "Internal temperature: " << record.Temp[0] << " °C" << std::endl;
        std::cout << "Internal humidity: " << record.Hum[0] << " °C" << std::endl;

        // TODO: date of external record
        for (size_t i = 1; i <= station.GetExternalSensors(); i++) {
            std::cout << "Sensor " << i << " temperature: " << record.Temp[i] << " °C" << std::endl;
            std::cout << "Sensor " << i << " humidity: " << record.Hum[i] << " %" << std::endl;
        }
    }
    catch (std::runtime_error const& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }

    return 0;
}