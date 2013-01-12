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

        auto record = station.history_last();
        std::cout << "Date/time last record: " << record.datetime << std::endl;
        std::cout << "Internal temperature: " << record.temperature[0] << " °C" << std::endl;
        std::cout << "Internal humidity: " << record.humidity[0] << " °C" << std::endl;

        for (size_t i = 1; i <= station.external_sensors(); i++) {
            std::cout << "Sensor " << i << " temperature: " << record.temperature[i] << " °C" << std::endl;
            std::cout << "Sensor " << i << " humidity: " << record.humidity[i] << " %" << std::endl;
        }
    }
    catch (std::runtime_error const& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }

    return 0;
}