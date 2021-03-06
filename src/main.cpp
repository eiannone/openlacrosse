//
// Configuration
//

// Standard library
#include <iostream>
#include <sstream>
#include <stdexcept>

// Boost
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <boost/algorithm/string.hpp>
#include <boost/exception/all.hpp>

// Local includes
#include "auxiliary.hpp"
#include "ws8610.hpp"

// Supported models
namespace Model
{
    enum Name
    {
        WS8610
    };
    std::istream& operator>>(std::istream& in, Name& name)
    {
        std::string token;
        in >> token;
        if (boost::iequals(token, "ws8610"))
            name = WS8610;
        else
            throw po::validation_error(
                po::validation_error::invalid_option_value,
                "Unknown model name");
        return in;
    }
};


//
// Main
//


namespace Formatting
{
    enum Mode
    {
        RAW,
        STRFTIME,
        CUSTOM
    };
};

std::string format_record(const Station::SensorRecord &record, time_t datetime, std::string type, unsigned int sensor, const std::string &format)
{
    std::stringstream format_stream;

    Formatting::Mode formatting = Formatting::RAW;
    for (size_t i = 0; i < format.size(); i++) {
        char current = format[i];
        if (formatting == Formatting::RAW) {
            switch (current) {
                case '%':
                    formatting = Formatting::STRFTIME;
                    break;
                default:
                    format_stream << current;
            }
        } else if (formatting == Formatting::STRFTIME) {
            switch (current) {
                case '#':
                    formatting = Formatting::CUSTOM;
                    break;
                default:
                    const std::string time_format = format.substr(i-1, 2).c_str();
                    char time_string[256];
                    if (strftime(time_string, 256, time_format.c_str(), localtime(&datetime)))
                    	format_stream << time_string;
                    else
                        throw std::runtime_error("invalid datetime formatting flag");

                    formatting = Formatting::RAW;
            }
        } else if (formatting == Formatting::CUSTOM) {
            switch (current) {
                case 'T':
                    if (record.temperature)
                        format_stream << *record.temperature;
                    else
                        format_stream << "-";
                    break;
                case 'H':
                    if (record.humidity)
                        format_stream << *record.humidity;
                    else
                        format_stream << "-";
                    break;
                case 't':
                	format_stream << type;
                    break;
                case 's':
                    format_stream << sensor;
                    break;
                default:
                    throw std::runtime_error("invalid sensor formatting flag");
            }
            formatting = Formatting::RAW;
        }
    }

    return format_stream.str();
}

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
            "model of the device\n"
            "supported models: WS8610")
        ("format,f",
            po::value<std::string>()
                ->default_value("%#t sensor %#s: %#T°C %#H%% at %c"),
            "how to format the sensor reading output\n"
            "supported formatting flags are:\n"
            " %#T: temperature in degrees Celsius\n"
            " %#H: humidity as percentage\n"
            " %#t: sensor type (internal or external)\n"
            " %#s: sensor number\n"
            " %-prefixed: strftime formatting\n")
		("dump",
			"dump memory contents after everything")
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
        clog(error) << "Error " << e.what() << std::endl;

        clog(info) << desc << std::endl;
        return 1;
    }

    // Display help
    if (vm.count("help")) {
        clog(info) << desc << std::endl;
        return 0;
    }

    // Notify the user of errors
    try {
        po::notify(vm);
    }
    catch (const std::exception &e) {
        clog(error) << "Invalid usage: " << e.what() << std::endl;

        clog(info) << desc << std::endl;
        return 1;
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
        clog(error) << "Error connecting to device: " << e.what() << std::endl;
        return 1;
    }
    

    //
    // Read data
    //
    
    try {
        auto record = station->history_last();

        clog(info) << format_record(record.internal, record.datetime, "internal", 1, vm["format"].as<std::string>()) << std::endl;
        for (size_t i = 0; i < record.external.size(); i++)
            clog(info) << format_record(record.external[i], record.datetime, "external", i+1, vm["format"].as<std::string>()) << std::endl;
    }
    catch (std::runtime_error const &e) {
        clog(error) << "Error reading data: " << e.what() << std::endl;
        return 1;
    }

    if (vm.count("dump")) {
    	std::vector<byte> memory = station->memory_dump();
    	clog(info) << hexdump(memory.data(), memory.size(), 16);
    }

    delete station;
    return 0;
}
