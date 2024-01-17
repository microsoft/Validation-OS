// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

//
// Bluetooth.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <bluetoothapis.h>
#include <iostream>
#include <string>
#include <wil/resource.h>
#include <wil/result.h>  

#pragma comment(lib, "Bthprops.lib")

void ScanBluetoothDevices() {
    BLUETOOTH_DEVICE_SEARCH_PARAMS btSearchParams = {
        sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
        TRUE, FALSE, TRUE, TRUE, TRUE, 2, NULL
    };
    BLUETOOTH_DEVICE_INFO btDeviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO) };

    HANDLE hDeviceFind = ::BluetoothFindFirstDevice(&btSearchParams, &btDeviceInfo);
    if (hDeviceFind == NULL || hDeviceFind == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to find devices. Error code: " + std::to_string(error));
    }

    wil::unique_handle deviceFindHandle(hDeviceFind);

    do {
        std::wcout << L"Found Bluetooth Device: " << btDeviceInfo.szName << std::endl;
    } while (::BluetoothFindNextDevice(hDeviceFind, &btDeviceInfo));
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::wcerr << "Commands: scan" << std::endl;
            return ERROR_INVALID_PARAMETER;
        }

        std::string command = argv[1];

        if (command == "scan") {
            ScanBluetoothDevices();
        }
        else {
            std::wcerr << "Invalid command or insufficient arguments." << std::endl;
            return ERROR_INVALID_COMMAND_LINE;
        }
    }
    catch (const wil::ResultException& e) {
        std::wcerr << "An unexpected error occurred: " << e.what() << std::endl;
        return ERROR_INTERNAL_ERROR;
    }
    catch (const std::exception& e) {
        std::wcerr << "An unexpected error occurred: " << e.what() << std::endl;
        return ERROR_INTERNAL_ERROR;
    }

    return ERROR_SUCCESS;
}
