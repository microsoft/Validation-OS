// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include <iostream>
#include <windows.h>           
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Sensors.h>
#include "GyrometersDemo.h"

using namespace winrt;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Sensors;
using namespace Windows::Foundation;

void GyrometersDemo::PrintGyrometerReading(const std::string& prefix, const winrt::Windows::Devices::Sensors::GyrometerReading& sensorReading)
{
	std::cout << prefix << " reading X: " << sensorReading.AngularVelocityX() << " - Y: " << sensorReading.AngularVelocityY() << " - Z: " << sensorReading.AngularVelocityZ() << std::endl;
	std::cout << prefix << " reading timestamp: " << sensorReading.Timestamp().time_since_epoch().count() << std::endl;

	// TODO: we can potentially add more information
}

GyrometersDemo::GyrometersDemo()
{
}

GyrometersDemo::~GyrometersDemo()
{
}

void GyrometersDemo::Start()
{
	auto gyrometer = Gyrometer::GetDefault();

	if (gyrometer)
	{
		gyrometer.ReadingChanged(
			[this](Gyrometer /*sensor*/, GyrometerReadingChangedEventArgs e)
			{
				auto reading = e.Reading();

				if (reading != nullptr)
				{
					PrintGyrometerReading("Reading", reading);
				}
				else
				{
					std::cerr << "Empty reading." << std::endl;
				}
			}
		);

		auto currentReading = gyrometer.GetCurrentReading();

		if (currentReading != nullptr)
		{
			PrintGyrometerReading("Current reading", currentReading);
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




