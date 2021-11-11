# Vega
A command-line tool to receive data from Ophir Vega Device

# Compile

## dependencies
* OphirCom: device driver
* clipp
* boost/range/adaptor/indexed

## using c++17
Note: this program is using some codes that are deprecated C++17.
# Before Using

## 1. Install Driver

First, Unplug the device from the computer. Double click `InstallOphirCom.bak` in the package, and confirm the consent prompt from UAC. (Or you also could right-click `InstallOphirCom.bak` and click on `Run as an administrator`) 

You will see a message from the windows registry server if success.

## 2. Open Command Prompt

There are two ways to open a Command Prompt in windows.
1. Open the Start menu, type `cmd`
2. Use `win+R`, type `cmd` and click `ok`

Note: The Command Prompt does not need to, and shouldn’t be run as an administrator. It might pose problems when you edit the data log files.

## 3. Change Directories

Use `cd /d` to Change Directories to the Vega.exe resides. 

(`/d` means changes the current drive at the same time.)

```DOS
cd /d "/path/to/Vega.exe"
```
For example, if `Vega.exe` is located in `C:\desktop\Ophir Vega\Vega.exe`, you can use

```DOS
cd /d "C:\desktop\Ophir Vega"
```

## 4. Run it!

And now you can use
```DOS
./Vega.exe [options]
```
to run the program

## 5. Uninstall Driver

Uninstall is not necessary after finishing the program. But if you want, unplug the device from the computer and double click the `RemoveOphirCom.bak` are all you have to do. 

# Usage
Using `./Vega.exe --help` to show this help information:

```PowerShell
PS C:\Desktop\Ophir Vega> .\Vega.exe --help

SYNOPSIS
  Vega.exe --help
  Vega.exe [--filter [<filter>]] [--range [<range>]] [--wavelength [<wavelength>]] [--MeasureMode][--no-save-setting] [--no-open-device] [--format [<Format>]] [-o <path>] [-q]

OPTIONS
  --help, -h                Show this message.
   You can use '--filter', '--range' or '--wavelength' to list options and use '--format' for more information of formatting output.

  --filter <filter>         Select the filter mode.
  --range <range>           Select the range.
  --wavelength <wavelength> Specify the wavelength.
  --MeasureMode             Select the MeasureMode.
  --no-save-setting         Do not save setting to Vega.
  --no-open-device          Do not open device.
  --format <Format>         Output data format. (default: 'yy-mm-dd HH:MM:SSttDATAttTIMESTAMP')
  -o, --output <path>       Output data to a file. (default: '.\Vega <Date>.txt')
  -q, --quiet               Run in quiet mode, will not print data info in command line.
```

# todo
* regenerate the project using CMake
* (maybe) turn this project into an installable exe