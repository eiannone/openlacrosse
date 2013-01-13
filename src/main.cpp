//
// Configuration
//

// Standard library
#include <iostream>
#include <stdexcept>

// Boost
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <boost/algorithm/string.hpp>
#include <boost/exception/all.hpp>

// Local includes
#include "auxiliary.h"
#include "ws8610.h"

// Supported models
namespace Model
{
    enum Name
    {
        WS8610
    };
};
std::istream& operator>>(std::istream& in, Model::Name& model)
{
    std::string token;
    in >> token;
    if (boost::iequals(token, "ws8610"))
        model = Model::WS8610;
    else
        throw po::validation_error(
            po::validation_error::invalid_option_value,
            "Unknown model");
    return in;
}


//
// Main
//

int main(int argc, char **argv)
{
    //
    // Command-line parameters
    //
    
    // Declare named options
    po::options_description desc("Program options:");
    desc.add_options()
        ("help,h",
            "produce help message")
        ("quiet,q",
            "only display errors and warnings")
        ("verbose,v",
            "display some more details")
        ("debug,d",
            "display even more details")
        ("device,d",
            po::value<std::string>()
                ->default_value("/dev/ttyS0"),
            "device to read from")
        ("model",
            po::value<Model::Name>()->required(),
            "model of the device (supported models: WS8610)")
        ("format,f",
            po::value<std::string>()->default_value(""),
            "how to format the sensor reading output")
    ;

    // Declare positional options
    po::positional_options_description pod;
    pod.add("port", -1);

    // Parse the options
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv)
            .options(desc).positional(pod).run(), vm);
    }
    catch (const std::exception &e) {
        std::cerr << "Error " << e.what() << std::endl;

        std::cout << desc << std::endl;
        return 1;
    }

    // Notify the user of errors
    try {
        po::notify(vm);
    }
    catch (const std::exception &e) {
        std::cerr << "Invalid usage: " << e.what() << std::endl;

        std::cout << desc << std::endl;
        return 1;
    }

    // Display help
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }


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
    // Connect
    //
    
    Station *station;
    try {
        switch (vm["model"].as<Model::Name>()) {
            case Model::WS8610:
                station = new WS8610(vm["device"].as<std::string>());
                break;
        }
    }
    catch (std::runtime_error const &e) {
        std::cerr << "Error connecting to device: " << e.what() << std::endl;
        return 1;
    }
    

    //
    // Read data
    //
    
    try {
        auto record = station->history_last();
        std::cout << "Date/time last record: " << record.datetime << std::endl;
        std::cout << "Internal temperature: " << record.internal.temperature << " °C" << std::endl;
        std::cout << "Internal humidity: " << record.internal.humidity << " °C" << std::endl;
        for (size_t i = 0; i < record.outdoor.size(); i++) {
            std::cout << "Sensor " << i << " temperature: " << record.outdoor[i].temperature << " °C" << std::endl;
            std::cout << "Sensor " << i << " humidity: " << record.outdoor[i].humidity << " %" << std::endl;
        }
    }
    catch (std::runtime_error const &e) {
        std::cerr << "Error reading data: " << e.what() << std::endl;
        return 1;
    }

    delete station;
    return 0;
}