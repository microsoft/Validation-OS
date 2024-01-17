// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include <windows.h>           
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Sensors.h>
#include "ProximitySensorsDemo.h"

using namespace winrt;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Sensors;
using namespace Windows::Foundation;

void ProximitySensorsDemo::PrintProximitySensorReading(const std::string& prefix, const winrt::Windows::Devices::Sensors::ProximitySensorReading& sensorReading)
{
	std::cout << prefix << " reading " << (sensorReading.IsDetected() ? "detected" : "not detected") << std::endl;
	std::cout << prefix << " reading timestamp: " << sensorReading.Timestamp().time_since_epoch().count() << std::endl;

	if (nullptr != sensorReading.DistanceInMillimeters())
	{
		std::cout << "Distance in millimeteres: " << sensorReading.DistanceInMillimeters().Value() << std::endl;
	}
	else
	{
		std::cerr << "No distance in millimeteres available";
	}
}

ProximitySensorsDemo::ProximitySensorsDemo() :
	watcher(nullptr)
{
}

ProximitySensorsDemo::~ProximitySensorsDemo()
{
	if (proximitySensorsReadingChangedToken)
	{
		watcher.Removed(proximitySensorsReadingChangedToken);
	}
	if (proximitySensorsAddedToken)
	{
		watcher.Removed(proximitySensorsAddedToken);
	}
}

void ProximitySensorsDemo::Start()
{
	watcher = DeviceInformation::CreateWatcher(ProximitySensor::GetDeviceSelector());
	proximitySensorsAddedToken = watcher.Added(
		[this](DeviceWatcher /*sender*/, DeviceInformation device)
		{
			std::cout << "Promity sensor added." << std::endl;
			std::cout << "ID: " << device.Id().c_str() << std::endl;

			auto foundSensor = ProximitySensor::FromId(device.Id());

			if (foundSensor)
			{
				std::cout << "Sensor found, listening to it." << std::endl;
				proximitySensorsReadingChangedToken = foundSensor.ReadingChanged(
					[this](ProximitySensor /*sensor*/, IProximitySensorReadingChangedEventArgs e)
					{
						auto reading = e.Reading();

						if (reading != nullptr)
						{
							PrintProximitySensorReading("Reading", reading);
						}
						else
						{
							std::cerr << "Empty reading." << std::endl;
						}
					}
				);

				auto currentReading = foundSensor.GetCurrentReading();

				if (currentReading != nullptr)
				{
					PrintProximitySensorReading("Current reading", currentReading);
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
	);
	watcher.Start();

}




