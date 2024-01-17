// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include <windows.h>           
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Sensors.h>
#include "MagnetometersDemo.h"

int main()
{
    winrt::init_apartment();

    std::cout << "Press ENTER to quit." << std::endl;

    MagnetometersDemo md;
    md.Start();

    // Block the main thread
    std::getchar();

    return 0;
}


