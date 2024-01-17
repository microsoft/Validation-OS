// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

class AccelerometersDemo
{
public:
	AccelerometersDemo();
	~AccelerometersDemo();

	void Start();

private:
	void PrintAccelerometerReading(const std::string& prefix, const winrt::Windows::Devices::Sensors::AccelerometerReading& sensorReading);

};
