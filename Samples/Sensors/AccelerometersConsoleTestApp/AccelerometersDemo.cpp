// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include <iostream>
#include <windows.h>           
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Sensors.h>
#include "AccelerometersDemo.h"

using namespace winrt;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Sensors;
using namespace Windows::Foundation;

void AccelerometersDemo::PrintAccelerometerReading(const std::string& prefix, const winrt::Windows::Devices::Sensors::AccelerometerReading& sensorReading)
{
	std::cout << prefix << " reading X: " << sensorReading.AccelerationX() << " - Y: " << sensorReading.AccelerationY() << " - Z: " << sensorReading.AccelerationZ() << std::endl;
	std::cout << prefix << " reading timestamp: " << sensorReading.Timestamp().time_since_epoch().count() << std::endl;

	// TODO: we can potentially add more information
}

AccelerometersDemo::AccelerometersDemo()
{
}

AccelerometersDemo::~AccelerometersDemo()
{
}

void AccelerometersDemo::Start()
{
	auto accelerometer = Accelerometer::GetDefault();	

	if (accelerometer)
	{
		accelerometer.ReadingChanged(
			[this](Accelerometer /*sensor*/, AccelerometerReadingChangedEventArgs e)
			{
				auto reading = e.Reading();

				if (reading != nullptr)
				{
					PrintAccelerometerReading("Reading", reading);
				}
				else
				{
					std::cerr << "Empty reading." << std::endl;
				}
			}
		);

		accelerometer.Shaken(
			[this](Accelerometer /*sensor*/, AccelerometerShakenEventArgs e)
			{
				std::cout << std::endl << "Accelerometer shaken" << std::endl;
			}
		);

		auto currentReading = accelerometer.GetCurrentReading();

		if (currentReading != nullptr)
		{
			PrintAccelerometerReading("Current reading", currentReading);
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




