//
// Configuration
//

// Standard library
#include <iostream>

// Boost
#include <boost/program_options.hpp>
namespace po = boost::program_options;

// Local includes
#include "auxiliary.h"
#include "ws8610.h"


//
// Main
//

int main(int argc, char **argv)
{
    //
    // Command-line parameters
    //
    
    // Declare named options
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("mode,m", po::value<std::string>()->required(), "program execution mode (possibilities: read, dump, reset)")
    ("quiet,q", "only display errors and warnings")
    ("verbose,v", "display some more details")
    ("debug,d", "display even more details")
    ("device,d", po::value<std::string>()->default_value("/dev/ttyS0"), "device to read from")
    ("format,f", po::value<std::string>()->default_value(""), "how to format the sensor reading output")
    ;

    // Declare positional options
    po::positional_options_description pod;
    pod.add("port", -1);

    // Parse the options
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
    options(desc).positional(pod).run(), vm);

    // Display help
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    // Notify the user of errors
    po::notify(vm);


    //
    // Initialization
    //
    
    // Set log-level
    if (vm.count("debug")) {
        logger.settings.threshold = trace;
        logger.settings.prefix_timestamp = true;
        logger.settings.prefix_level = true;
    } else if (vm.count("verbose"))
        logger.settings.threshold = debug;
    else if (vm.count("quiet"))
        logger.settings.threshold = warning;


    //
    // Execute program modes
    //
    
    try {
        WS8610 station(vm["device"].as<std::string>());

        std::string mode = vm["mode"].as<std::string>();
        if (mode == "read") {
            auto record = station.history_last();
            std::cout << "Date/time last record: " << record.datetime << std::endl;
            std::cout << "Internal temperature: " << record.internal.temperature << " °C" << std::endl;
            std::cout << "Internal humidity: " << record.internal.humidity << " °C" << std::endl;
            for (size_t i = 0; i < record.outdoor.size(); i++) {
                std::cout << "Sensor " << i << " temperature: " << record.outdoor[i].temperature << " °C" << std::endl;
                std::cout << "Sensor " << i << " humidity: " << record.outdoor[i].humidity << " %" << std::endl;
            }
        }
        else {
            throw po::validation_error(po::validation_error::invalid_option_value, "program execution mode");
        }
    }    
    catch (std::runtime_error const& e) {
        std::cerr << "Runtime exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}