// Sample code for using the OphirLMMeasurement.CoLMMeasurement COM object C++
// wrapper. See OphirLMMeasurement.h for more details.

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

#define NOMINMAX
//#pragma warning(disable : 4996)
#include <Windows.h>

#include <signal.h>
#include <stdlib.h>

#include "Vega.h"
#include "__DATE.h"
#include "clipp.h"
using namespace clipp;

struct config
{
    std::string Filter;
    std::string Range;
    std::string Wavelength;
    std::filesystem::path output = ".";
    std::string format           = "yy-mm-dd HH:MM:SSttDATAttSTATUSttTIMESTAMP";
} conf;
void signalExitFunction(int signal);

int main(int argc, char **argv)
{
    signal(SIGINT, signalExitFunction);
    signal(SIGABRT, signalExitFunction);
    enum mode : unsigned short
    {
        help          = 0x00,
        run           = 1 << 0,
        rangeHelp     = 1 << 1,
        waveHelp      = 1 << 2,
        filterHelp    = 1 << 3,
        FormatHelp    = 1 << 4,
        MeasureHelp   = 1 << 5,
        NoSaveSetting = 1 << 6,
        NoOpenDevice  = 1 << 7,
        Quite         = 1 << 8,
    };
    unsigned short selected = mode::run;
    auto cli                = with_prefixes_short_long(
        "-", "--",
        (option("help", "h").call([&]() { selected = mode::help; })) % "Show this message. \n You can use \'--filter\', \'--range\' or \'--wavelength\' to list options and use \'--format\' for more information of formatting output."
            | ((option("filter").call([&]() { selected |= mode::filterHelp; })
                & opt_value("filter", conf.Filter).call([&]() { selected &= ~mode::filterHelp; }))
                   % "Select the filter mode.",
               (option("range").call([&]() { selected |= mode::rangeHelp; })
                & opt_value("range", conf.Range).call([&]() { selected &= ~mode::rangeHelp; }))
                   % "Select the range. ",
               (option("wavelength").call([&]() { selected |= mode::waveHelp; })
                & opt_value("wavelength", conf.Wavelength).call([&]() { selected &= ~mode::waveHelp; }))
                   % "Specify the wavelength.",
               (option("MeasureMode").call([&]() { selected |= mode::MeasureHelp; }))
                   % "Select the MeasureMode.",
               (option("no-save-setting").call([&]() { selected |= mode::NoSaveSetting; }))
                   % "Do not save setting to Vega.",
               (option("no-open-device").call([&]() { selected |= mode::NoOpenDevice; }))
                   % "Do not open device.",
               (option("format").call([&]() { selected |= mode::FormatHelp; })
                & opt_value("Format", conf.format))
                   % "Output data format. (default: \'yy-mm-dd HH:MM:SSttDATAttTIMESTAMP\')",
               (option("o", "output") & value("path", conf.output))
                   % "Output data to a file. (default: \'.\\Vega <Date>.txt\')",
               (option("q", "quiet").call([&]() { selected |= mode::Quite; }) % "Run in quiet mode, will not print data info in command line.")));

    auto fmt = doc_formatting{}
                   .first_column(2)  //left border column for text body
                   .doc_column(28)   //column where parameter docstring starts
                   .last_column(100) //right border column for text body
        ;
    std::string programName = std::filesystem::path(argv[0]).filename().string() + '\0';
    memcpy(argv[0], programName.c_str(), sizeof(char) * programName.size());
    if (!parse(argc, argv, cli))
    {
        std::cout << "Error param.\n"
                  << "Try \'" << argv[0] << " --help\' for more information.\n";
        return 0;
    }
    if (selected == mode::help)
    {
        std::cout << make_man_page(cli, argv[0], fmt);
        return 0;
    }
    time_t tt = time(nullptr);

    if (selected & mode::FormatHelp)
    {
        auto formatHelp = "The following specifier could be used for formatting output"
                          % (command("--format"),
                             (
                                 command("yy") % "year"
                                 | command("mm") % "month"
                                 | command("dd") % "day"
                                 | command("HH") % "Hour"
                                 | command("MM") % "Minute"
                                 | command("SS") % "Second"
                                 | command("tt") % " Horizontal-tab character "
                                 | command("DATA") % "data in Watt"
                                 | command("STATUS") % "data receiving status"
                                 | command("TIMESTAMP") % "data timestamp in ms")
                                 % "specifier: ");
        auto formatfmt = doc_formatting{}
                             .first_column(2)
                             .doc_column(28)
                             .last_column(60);
        char buf[255];
        std::strftime(buf, sizeof(buf), "This program was built in \"yy-mm-dd HH:MM:SSttBy JHees\" \n\t-> This program was built in \"%y-%m-%d %H:%M:%S %tBy JHees\"\n", &BUILD_TM);
        std::string excample;
        excample.append(buf);
        std::strftime(buf, sizeof(buf), "Today is ttyy/mm/dd, ttand now time is ttHH,MM,SS\n\t-> Today is %t%y/%m/%d, %tand now time is %t%H,%M,%S\n", localtime(&tt));
        excample.append(buf);

        std::cout << make_man_page(formatHelp, argv[0], formatfmt)
                         .append_section("EXCAMPLES", excample);

        return 0;
    }

    conf.output = std::filesystem::absolute(conf.output);
    auto ext    = conf.output.has_extension() ? conf.output.extension() : "txt";
    conf.output.replace_extension(ext);
    if (std::filesystem::exists(conf.output))
    {
        if (std::filesystem::is_directory(conf.output))
        {
            char fileName[20];
            std::strftime(fileName, sizeof(fileName), "%y-%m-%d", localtime(&tt));
            conf.output /= "Vega " + std::string(fileName);
            conf.output.replace_extension(ext);
        }
        if (std::filesystem::is_regular_file(conf.output))
        {
            std::string str = conf.output.stem().generic_string();
            do
            {
                std::smatch sm;
                if (std::regex_search(str, sm, std::regex("[ ]-[ ]([\\d]{1,})$")))
                {
                    str = std::regex_replace(str, std::regex("[ ]-[ ]([\\d]{1,})$"), " - " + std::to_string(std::stoi(sm[1]) + 1));
                }
                else
                {
                    str.append(" - 1");
                }
                conf.output.replace_filename(str);
                conf.output.replace_extension(ext);
            } while (std::filesystem::exists(conf.output));
        }
    }
    std::ofstream file;
    file.open(conf.output);
    if (!file.is_open())
        std::cout << "cannot open file." << conf.output << '\n'
                  << "Exiting." << '\n';

    std::cout << "The following data logs will be recored in: \n"
              << "file: " << conf.output << '\n'
              << "format: " << conf.format << "\n\n";

    conf.format = std::regex_replace(conf.format, std::regex("([ymdHMS]){2}|[\\s]*t(t)[\\s]*"), "%$1$2");

    Vega device;
    try
    {
        if (!device.init())
            exit(-1);

        if (selected & (mode::rangeHelp | mode::waveHelp | mode::filterHelp | mode::MeasureHelp))
        {
            (selected & mode::rangeHelp) ? device.print_options(Vega::configuration::Ranges) : void(0);
            (selected & mode::waveHelp) ? device.print_options(Vega::configuration::Wavelengths) : void(0);
            (selected & mode::filterHelp) ? device.print_options(Vega::configuration::Filter) : void(0);
            (selected & mode::MeasureHelp) ? device.print_options(Vega::configuration::MeasurementMode) : void(0);
            //device.print_options(Vega::configuration::PulseLengths);
            //device.print_options(Vega::configuration::Diffuser);
            //device.print_options(Vega::configuration::Threshold);
            return 0;
        }

        std::wcout << L"Vega's settings: \n";
        device.print_options(Vega::configuration::MeasurementMode, false);
        conf.Filter.empty() ? device.print_options(Vega::configuration::Filter, false)
                            : device.set_config(Vega::configuration::Filter, conf.Filter); // must set filter first
        conf.Range.empty() ? device.print_options(Vega::configuration::Ranges, false)
                           : device.set_config(Vega::configuration::Ranges, conf.Range);
        conf.Wavelength.empty() ? device.print_options(Vega::configuration::Wavelengths, false)
                                : device.set_config(Vega::configuration::Wavelengths, conf.Wavelength);
        (selected & mode::NoSaveSetting) ? void(0) : device.save_config(); // save setting
        if (selected & mode::NoOpenDevice)
            return 0;

        unsigned int dataNum = 0;
        size_t size          = 0;
        device.registerDataReadyCallback([&](long hDevice, long channel) {
            std::vector<double> values;
            std::vector<double> timestamps;
            std::vector<OphirLMMeasurement::Status> statuses;
            device.OphiLM().GetData(hDevice, channel, values, timestamps, statuses);

            for (size_t i = 0; i < values.size(); ++i)
            {
                ++dataNum;
                tt = time(nullptr) - time_t((timestamps[timestamps.size() - 1] - timestamps[i]) / 1e3);
                char buf[255];
                std::strftime(buf, sizeof(buf), conf.format.c_str(), localtime(&tt));
                std::string ret(buf);
                std::stringstream data;
                data << std::to_string(dataNum) << '\t' << std::scientific << std::setprecision(4) << values[0];
                ret = std::regex_replace(ret, std::regex("DATA"), data.str());
                data.str("");
                data << ws2s(device.OphiLM().StatusString(statuses[i]));
                ret = std::regex_replace(ret, std::regex("STATUS"), data.str());
                data << std::fixed << std::setprecision(3) << timestamps[i];
                ret = std::regex_replace(ret, std::regex("TIMESTAMP"), data.str());
                file << ret << '\n';

                if (selected & mode::Quite)
                {
                    std::string str = std::string(size, '\b') + std::to_string(dataNum) + " data received.";
                    std::cout << str;
                    size = str.size();
                }
                else
                    std::cout << ret << '\n';
            }
        });

        std::cout << std::string("\nStart to run") + ((selected & mode::Quite) ? " in quite mode.\n" : ".\n\n");
        device.open_stream();
        device.run();
    }
    catch (const _com_error &e)
    {
        std::cout << "Error 0x" << std::hex << e.Error() << " "
                  << e.Description() << "\n";
        file.close();
        exit(-1);
        // 0x00000000(S_OK) : No Error
        // 0x80004001(E_NOTIMPL) : Function Not Implemented
        // 0x80070057(E_INVALIDARG) : Invalid Argument
        // 0x80004005(E_FAIL) : Unspecified Failure
        // 0x80040200 : Device not opened
        // 0x80040201 : Device Already Opened
        // 0x80040202 : Drivers cannot be loaded
        // 0x80040203 : Load File Missing
        // 0x80040300 : Device Failed
        // 0x80040301 : Device Firmware is Incorrect
        // 0x80040302 : Sensor Failed
        // 0x80040303 : Sensor Firmware is Incorrect
        // 0x80040304 : Bad Device Handle
        // 0x80040305 : Bad Sensor Channel
        // 0x80040306 : This Sensor is Not Supported
        // 0x80040307 : Not Applicable in this Device
        // 0x80040308 : The Device is no longer Available
        // 0x80040400 : Save To Sensor Failed
        // 0x80040401 : Param Error
        // 0x80040402 : Failed to create Safe Array
        // 0x80040403 : Not Applicable in this Sensor
        // 0x80040404 : Value Out of Range
        // 0x80040405 : Command Failed
        // 0x80040500 : Stream Mode Not Started
        // 0x80040501 : A channel is in Stream Mode
    }
    signalExitFunction(SIGINT);
    return 0;
}

void signalExitFunction(int signal)
{
    std::cout << "\n\n"
              << "The data logs was recored in: \n"
              << "file: " << conf.output << '\n'
              << "format: " << conf.format << "\n\n"
              << "Exiting." << '\n';
    if (signal == SIGINT)
        exit(0);
    else
        exit(-1);
}