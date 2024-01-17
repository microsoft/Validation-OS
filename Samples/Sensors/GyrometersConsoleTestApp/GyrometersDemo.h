// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

class GyrometersDemo
{
public:
	GyrometersDemo();
	~GyrometersDemo();

	void Start();

private:
	void PrintGyrometerReading(const std::string& prefix, const winrt::Windows::Devices::Sensors::GyrometerReading& sensorReading);

};
