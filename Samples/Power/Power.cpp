// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

//
// Power.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WIN32_WINNT 0x0600
#include <Windows.h>
#include <PowrProf.h>
#include <iostream>
#include <tchar.h>
#include <wil/resource.h>

#pragma comment(lib, "PowrProf.lib")

int __cdecl main()
{
    // Define and initialize our return variables.
    ULONG bufSize = MAX_PATH * sizeof(WCHAR);
    ULONG uIndex = 0;
    BOOLEAN bRet = FALSE;

    // Open the device list, querying all devices
    if (!::DevicePowerOpen(0))
    {
        std::wcout << "ERROR: The device database failed to initialize.\n";
        return ERROR_DATABASE_FAILURE;
    }

    // Enumerate the device list, searching for devices that support 
    // waking from either the S1 or S2 sleep state and are currently 
    // present in the system, and not devices that have drivers 
    // installed but are not currently attached to the system, such as 
    // in a laptop docking station.

    wil::unique_hlocal pRetBuf(LocalAlloc(LPTR, bufSize));

    while (NULL != pRetBuf &&
        0 != (bRet = ::DevicePowerEnumDevices(uIndex,
            DEVICEPOWER_FILTER_DEVICES_PRESENT,
            PDCAP_WAKE_FROM_S1_SUPPORTED | PDCAP_WAKE_FROM_S2_SUPPORTED,
            reinterpret_cast<PBYTE>(pRetBuf.get()),
            &bufSize)))
    {
        std::wcout << L"Device name: " << reinterpret_cast<LPWSTR>(pRetBuf.get()) << L"\n";

        // For the devices we found that have support for waking from 
        // S1 and S2 sleep states, disable them from waking the system.
        bRet = (0 != DevicePowerSetDeviceState((LPCWSTR)pRetBuf.get(),
            DEVICEPOWER_CLEAR_WAKEENABLED,
            NULL));

        if (0 != bRet)
        {
            std::wcout << "Warning: Failed to set device state.\n";
        }
        else
        {
            std::wcout << L"Successfully disabled wake capability for " <<reinterpret_cast<LPWSTR>(pRetBuf.get()) << L".\n";
        }
        uIndex++;
    }
    // Close the device list.
    ::DevicePowerClose();

    return ERROR_SUCCESS;
}