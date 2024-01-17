// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include <windows.h>           
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Sensors.h>
#include "MagnetometersDemo.h"
#include <iostream>

using namespace winrt;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Sensors;
using namespace Windows::Foundation;

void MagnetometersDemo::PrintMagnetometerReading(const std::string& prefix, const winrt::Windows::Devices::Sensors::MagnetometerReading& sensorReading)
{
	std::cout << prefix << " reading X: " << sensorReading.MagneticFieldX() << " - Y: " << sensorReading.MagneticFieldY() << " - Z: " << sensorReading.MagneticFieldZ() << std::endl;
	std::cout << prefix << " reading timestamp: " << sensorReading.Timestamp().time_since_epoch().count() << std::endl;

	// TODO: we can potentially add more information
}

void MagnetometersDemo::Start()
{
	auto magnetometer = Magnetometer::GetDefault();
	
	if (magnetometer)
	{
		magnetometer.ReadingChanged(
			[this](Magnetometer /*sensor*/, IMagnetometerReadingChangedEventArgs e)
			{
				auto reading = e.Reading();

				if (reading != nullptr)
				{
					PrintMagnetometerReading("Reading", reading);
				}
				else
				{
					std::cerr << "Empty reading." << std::endl;
				}
			}
		);

		auto currentReading = magnetometer.GetCurrentReading();

		if (currentReading != nullptr)
		{
			PrintMagnetometerReading("Current reading", currentReading);
		}
		else
		{
			std::cerr << "Empty current reading" << std::endl;
		}
	}
	else
	{
		std::cerr << "Sensor not found." << std::endl;
	}
}




