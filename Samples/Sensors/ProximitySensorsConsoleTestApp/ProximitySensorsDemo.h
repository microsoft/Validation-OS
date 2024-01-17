// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

class ProximitySensorsDemo
{
public:
	ProximitySensorsDemo();
	~ProximitySensorsDemo();

	void Start();

private:
	winrt::Windows::Devices::Enumeration::DeviceWatcher watcher;
	winrt::event_token proximitySensorsAddedToken;
	winrt::event_token proximitySensorsReadingChangedToken;

	void PrintProximitySensorReading(const std::string& prefix, const winrt::Windows::Devices::Sensors::ProximitySensorReading& sensorReading);

};
