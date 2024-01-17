// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

//
//  Command line parsing logic
//
struct CommandLineSwitch
{
    enum CommandLineSwitchType
    {
        SwitchTypeNone,
        SwitchTypeInteger,
        SwitchTypeString,
    };

    LPCWSTR SwitchName;
    LPCWSTR SwitchHelp;
    CommandLineSwitchType SwitchType;
    void **SwitchValue;
    bool SwitchValueOptional;
};

bool ParseCommandLine(int argc, wchar_t *argv[], const CommandLineSwitch Switches[], size_t SwitchCount);
