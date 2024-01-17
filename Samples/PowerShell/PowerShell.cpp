// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

//
// PowerShell.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <iostream>
#include <string>

std::string ExecutePowerShellCommand(const std::string& command) {
    std::string data;
    FILE* stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    std::string cmd = "powershell -Command \"" + command + "\"";

    stream = _popen(cmd.c_str(), "r");

    if (stream) {
        while (!feof(stream)) {
            if (fgets(buffer, max_buffer, stream) != nullptr) {
                data.append(buffer);
            }
        }
        _pclose(stream);
    }

    return data;
}

void PrintUsage() {
    std::cout << "Usage: PowerShell.exe [command]\n"
        << "Commands:\n"
        << "  /GetDisk               Lists disk information.\n"
        << "  /GetCmdlets            Lists all PowerShell cmdlets.\n"
        << "  /GetModules            Lists all available PowerShell modules.\n"
        << "  /GetProviders          Lists all PowerShell providers.\n"
        << "  /CheckVersion          Checks the installed PowerShell version.\n"
        << "  /GetSystemInformation  Checks the system information.\n"
        << "  /h or /?               Prints this help message.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    std::string arg = argv[1];
    try {
        if (arg == "/GetDisk") {
            std::string getDisk = ExecutePowerShellCommand("Get-Disk | Format-Table -AutoSize | Out-String -Width 4096");
            std::cout << "Get-Disk:\n" << getDisk << std::endl;
        }
        else if (arg == "/GetCmdlets") {
            std::string cmdlets = ExecutePowerShellCommand("Get-Command | Format-Table -AutoSize | Out-String -Width 4096");
            std::cout << "Cmdlets:\n" << cmdlets << std::endl;
        }
        else if (arg == "/GetModules") {
            std::string modules = ExecutePowerShellCommand("Get-Module -ListAvailable | Format-Table -AutoSize | Out-String -Width 4096");
            std::cout << "Modules:\n" << modules << std::endl;
        }
        else if (arg == "/GetProviders") {
            std::string providers = ExecutePowerShellCommand("Get-PSProvider | Format-Table -AutoSize | Out-String -Width 4096");
            std::cout << "Providers:\n" << providers << std::endl;
        }
        else if (arg == "/CheckVersion") {
            std::string providers = ExecutePowerShellCommand("$PSVersionTable.PSVersion | Format-Table -AutoSize | Out-String -Width 4096");
            std::cout << "PowerShell Version:\n" << providers << std::endl;
        }
        else if (arg == "/CheckVersion") {
            std::string version = ExecutePowerShellCommand("$PSVersionTable.PSVersion | Format-Table -AutoSize | Out-String -Width 4096");
            std::cout << "PowerShell Version:\n" << version << std::endl;
        }
        else if (arg == "/GetSystemInformation") {
            std::string information = ExecutePowerShellCommand("Get-CimInstance Win32_Processor | Format-Table -AutoSize | Out-String -Width 4096");
            std::cout << "System Information:\n" << information << std::endl;
        }
        else if (arg == "/h" || arg == "/?") {
            PrintUsage();
        }
        else {
            std::cerr << "Unknown command. Use /h or /? for help." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}
