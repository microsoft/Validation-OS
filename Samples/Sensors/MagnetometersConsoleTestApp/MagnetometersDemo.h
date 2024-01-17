// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

class MagnetometersDemo
{
public:
	void Start();

private:
	void PrintMagnetometerReading(const std::string& prefix, const winrt::Windows::Devices::Sensors::MagnetometerReading& sensorReading);

};
