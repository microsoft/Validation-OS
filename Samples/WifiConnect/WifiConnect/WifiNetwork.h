#pragma once
#include <string>

// Plain data model for a discovered WiFi network.
// Owned by WifiManager; consumed read-only by the UI layer.
struct WifiNetwork
{
    std::wstring ssid;
    DWORD        signalQuality = 0;   // 0–100 (percentage)
    bool         isConnected   = false;
    bool         isSecured     = false;
};
