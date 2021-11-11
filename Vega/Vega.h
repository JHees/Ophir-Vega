#pragma once
#include "OphirLMMeasurement.h"
#include "util.h"
#include <boost/range/adaptor/indexed.hpp>
#include <comdef.h>
#include <iomanip>
#include <iostream>
#include <regex>

class Vega
{
public:
    enum class configuration
    {
        Diffuser,
        Filter,
        //LowFreqPowerPulseFreq,
        MeasurementMode,
        //PulsedPowerPulseWidth,
        PulseLengths,
        Ranges,
        Threshold,
        Wavelengths,
        //WavelengthsExtra
        //todo AverageTime need to use User Command
    };
    std::wstring get_conf_name(configuration config)
    {
        switch (config)
        {
        case configuration::Diffuser:
            return L"Diffuser";
            break;
        case configuration::Filter:
            return L"Filter";
            break;
        case configuration::MeasurementMode:
            return L"MeasurementMode";
            break;
        case configuration::PulseLengths:
            return L"PulseLengths";
            break;
        case configuration::Ranges:
            return L"Ranges";
            break;
        case configuration::Threshold:
            return L"Threshold";
            break;
        case configuration::Wavelengths:
            return L"Wavelengths";
            break;
        }
        return L"";
    }
    struct status
    {
        long index;
        std::vector<std::wstring> options;
        union
        {
            struct
            {
                bool mod;
                long min, max;
            } WavelengthsExtraData;
            struct
            {
                double value, min, max;
            } LowFreqPowerPulseFreqData;
            struct
            {
                long value, min, max;
            } PulsedPowerPulseWidthData;
        } data;
    };

public:
    Vega(){};
    ~Vega()
    {
        OphirLM.StopAllStreams(); // stop measuring
        OphirLM.CloseAll();       // close device
    }
    bool init(size_t serialIndex = 0)
    {
        // start measuring on a device
        std::vector<std::wstring> serialNumbers;
        std::wstring headSN, headType, headName, version;
        std::wstring deviceName, romVersion, serialNumber;

        // Scan for connected Devices
        OphirLM.ScanUSB(serialNumbers);
        std::wcout << L"Opening Device.\n";
        std::wcout << serialNumbers.size() << L" Ophir USB devices found. \n";

        if (serialNumbers.size() > 0)
        {
            // open device
            OphirLM.OpenUSBDevice(serialNumbers[serialIndex].c_str(), hDevice);
            //std::wcout << L"Ophir device opened with handle " << hDevice << L".\n\n";

            // get device info
            OphirLM.GetDeviceInfo(hDevice, deviceName, romVersion, serialNumber);
            std::wcout << L"Device Name:   " << deviceName << L" \n";
            std::wcout << L"Rom Version:   " << romVersion << L" \n";
            std::wcout << L"Serial Number: " << serialNumber << L" \n";
            // get sensor info of first device
            OphirLM.GetSensorInfo(hDevice, channel, headSN, headType, headName);
            std::wcout << L"Head name:          " << headName << L" \n";
            std::wcout << L"Head type:          " << headType << L" \n";
            std::wcout << L"Head serial number: " << headSN << L" \n";
            std::wcout << L"Open Device success.\n\n";
            return true;
        }
        else
        {

            std::wcout << L"Open Device FAILED.\n\n";
            return false;
        }
    }

    void registerDataReadyCallback(std::function<void(long, long)> callback) //!need to call OphirLM().GetData here
    {
        DataReadyCallback = callback;
    }
    void registerPlugAndPlayCallback(std::function<void(void)> callback)
    {
        PlugAndPlayCallback = callback;
    }
    void open_stream() //todo streamMode
    {
        //OphirLM.ConfigureStreamMode(hDevice, channel, 1, 2000);
        //OphirLM.ConfigureStreamMode(hDevice, channel, 0, 1);
        //OphirLM.ConfigureStreamMode(hDevice, channel, 2, 0);

        OphirLM.RegisterPlugAndPlay(PlugAndPlayCallback);
        OphirLM.RegisterDataReady(DataReadyCallback);
        OphirLM.StartStream(hDevice, channel);
    }
    void run()
    {
        // A message loop is necessary to receive events.
        // An alternative is to call GetData in a loop (with a small delay in the
        // loop) and not use events at all.
        MSG Msg;
        while (GetMessage(&Msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }
    void print_options(configuration config, bool is_printAllOptions = true)
    {
        auto status       = get_options(config);
        std::wstring name = get_conf_name(config);
        if (status.index != -1)
        {
            std::wcout << name << L" : " << status.options[status.index] << L'\n';
            if (is_printAllOptions)
            {
                for (size_t i = 0; i < status.options.size(); ++i)
                    std::wcout << " options[" << i << "]: " << status.options[i] << L'\n';
                if (config == configuration::Wavelengths)
                    if (status.data.WavelengthsExtraData.mod)
                        std::wcout << L"you may choose within options or specify the wavelength between " << status.data.WavelengthsExtraData.min << L" and " << status.data.WavelengthsExtraData.max << L"\n";
                    else
                        std::wcout << L"you could specify the wavelength between " << status.data.WavelengthsExtraData.min << L" and " << status.data.WavelengthsExtraData.max << L"\n";
                std::wcout << "\n";
            }
        }
        else
            std::wcout << "Vega have not " << name << " options" << L'\n';
    }
    void set_config(configuration config, std::string input)
    {
        long index          = -1;
        Vega::status status = get_options(config);
        if (std::regex_match(input, std::regex("\\d+")) && (size_t)std::stol(input) < status.options.size())
            index = std::stol(input);
        else
        {
            auto matchOption = [&](const std::string &in) -> long {
                for (auto const &op : status.options | boost::adaptors::indexed(0))
                {
                    if (std::regex_match(in, std::regex(ws2s(op.value()), std::regex_constants::icase)))
                    {
                        return op.index();
                    }
                }
                return -1;
            };
            if (index = matchOption(input); index == -1)
            {

                switch (config)
                {
                case configuration::Diffuser:
                case configuration::MeasurementMode:
                case configuration::PulseLengths:
                case configuration::Threshold:
                    break;
                case configuration::Filter:
                    break;
                case configuration::Ranges:
                    do
                    {
                        for (auto &i : status.options | boost::adaptors::indexed(0))
                        {
                            std::wsmatch wsm;
                            std::smatch sm;

                            if (std::regex_search(i.value(), wsm, std::wregex(L"(\\d*(\\.\\d+)?)([mun])(w|W)"))
                                && std::regex_match(input, sm, std::regex("(\\d*(\\.\\d+)?)([mun])(w|W)")))
                            {
                                float option = std::stof(ws2s(wsm[1]))
                                               * float(wsm[3] == L"m" ? 1e-3 : wsm[3] == L"u" ? 1e-6
                                                                           : wsm[3] == L"n"   ? 1e-9
                                                                                              : 1);
                                float input = std::stof(sm[1])
                                              * float(sm[3] == "m" ? 1e-3 : sm[3] == "u" ? 1e-6
                                                                        : sm[3] == "n"   ? 1e-9
                                                                                         : 0);
                                if (option == input)
                                {
                                    index = i.index();
                                    break;
                                }
                            }
                        }
                    } while (0);
                    break;
                case configuration::Wavelengths:
                    do
                    {
                        std::smatch sm;
                        if (std::regex_search(input, sm, std::regex("(\\d+)(nm)?")))
                        {
                            index = matchOption(sm[1]);
                            if (int wavelength = std::stoi(sm[1]); index == -1 && status.data.WavelengthsExtraData.mod && status.data.WavelengthsExtraData.min < wavelength && wavelength < status.data.WavelengthsExtraData.max)
                            {
                                try
                                {
                                    OphirLM.AddWavelength(hDevice, channel, wavelength);
                                    index = status.options.size();
                                    status.options.push_back(std::to_wstring(wavelength));
                                }
                                catch (const _com_error &e)
                                {
                                    if (e.Error() == 0x80040403)
                                    {
                                        index = status.options.size() - 1;
                                        OphirLM.DeleteWavelength(hDevice, channel, index);
                                        OphirLM.AddWavelength(hDevice, channel, wavelength);
                                        status.options[index] = std::to_wstring(wavelength);
                                    }
                                    else
                                        throw e;
                                    // 0x00000000(S_OK) : No Error
                                    // 0x80040200 : Device not opened
                                    // 0x80040304 : Bad Device Handle
                                    // 0x80040305 : Bad Sensor Channel
                                    // 0x80040308 : The Device is no longer Available
                                    // 0x80040403 : Not Applicable in this Sensor
                                    // 0x80040404 : Wavelength Out of Range
                                    // 0x80040405 : Command Failed
                                    // 0x80040501 : A channel is in Stream Mode
                                }
                            }
                        }
                    } while (0);
                    break;
                }
            }
        }
        if (index == -1)
        {
            std::wcout << get_conf_name(config) << L" : unknown option was given to set config.\n";
            return;
        }
        else
            std::wcout << get_conf_name(config) << L" : " << status.options[index] << L"\n";
        switch (config)
        {
        case configuration::Diffuser:
            OphirLM.SetDiffuser(hDevice, channel, index);
            break;
        case configuration::Filter:
            OphirLM.SetFilter(hDevice, channel, index);
            break;
        case configuration::MeasurementMode:
            OphirLM.SetMeasurementMode(hDevice, channel, index);
            break;
        case configuration::PulseLengths:
            OphirLM.SetPulseLength(hDevice, channel, index);
            break;
        case configuration::Ranges:
            OphirLM.SetRange(hDevice, channel, index);
            break;
        case configuration::Threshold:
            OphirLM.SetThreshold(hDevice, channel, index);
            break;
        case configuration::Wavelengths:
            OphirLM.SetWavelength(hDevice, channel, index);
            break;
        }
    }
    void save_config()
    {
        return OphirLM.SaveSettings(hDevice, channel);
    }

private:
    status get_options(configuration config)
    {
        status ret;
        switch (config)
        {
        case configuration::Diffuser:
            OphirLM.GetDiffuser(hDevice, channel, ret.index, ret.options);
            break;
        case configuration::Filter:
            OphirLM.GetFilter(hDevice, channel, ret.index, ret.options);
            break;
        case configuration::MeasurementMode:
            OphirLM.GetMeasurementMode(hDevice, channel, ret.index, ret.options);
            break;
        case configuration::PulseLengths:
            OphirLM.GetPulseLengths(hDevice, channel, ret.index, ret.options);
            break;
        case configuration::Ranges:
            OphirLM.GetRanges(hDevice, channel, ret.index, ret.options);
            break;
        case configuration::Threshold:
            OphirLM.GetThreshold(hDevice, channel, ret.index, ret.options);
            break;
        case configuration::Wavelengths:
            OphirLM.GetWavelengths(hDevice, channel, ret.index, ret.options);
            OphirLM.GetWavelengthsExtra(hDevice, channel, ret.data.WavelengthsExtraData.mod, ret.data.WavelengthsExtraData.min, ret.data.WavelengthsExtraData.max);

            break;
        }
        return ret;
    }

private:
    std::function<void(long, long)> DataReadyCallback = [](long hDevice, long channel) {
        std::vector<double> values;
        std::vector<double> timestamps;
        std::vector<OphirLMMeasurement::Status> statuses;
        OphirLM.GetData(hDevice, channel, values, timestamps, statuses);
        for (size_t i = 0; i < values.size(); ++i)
            std::wcout << L"Timestamp: " << std::fixed << std::setprecision(3)
                       << timestamps[i] << L" Reading: " << std::scientific << values[i]
                       << L" Status: " << OphirLM.StatusString(statuses[i]) << L"\n";
    };
    std::function<void(void)> PlugAndPlayCallback = []() {
        std::wcout << L"Device has been removed from the USB. \n";
    };

public:
    OphirLMMeasurement &OphiLM()
    {
        return Vega::OphirLM;
    }

private:
    static OphirLMMeasurement OphirLM;
    static struct CoInitializer
    {
        CoInitializer() { CoInitialize(nullptr); }
        ~CoInitializer() { CoUninitialize(); }
    } initializer; // must call for COM initialization and deinitialization
private:
    long hDevice = 0, channel = 0;
};
Vega::CoInitializer Vega::initializer; // call before initialize OphirLM
OphirLMMeasurement Vega::OphirLM;