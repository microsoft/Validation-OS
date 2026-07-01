#pragma once
#include <windows.h>
#include <wlanapi.h>
#include <vector>
#include <string>
#include "WifiNetwork.h"
#include "resource.h"
#include "Utils.h"

// Wraps the Windows WLAN API.
// One instance per application; call Initialize() before any other method.
class WifiManager
{
public:
    WifiManager()  = default;
    ~WifiManager() { Shutdown(); }

    // Non-copyable
    WifiManager(const WifiManager&)            = delete;
    WifiManager& operator=(const WifiManager&) = delete;

    bool Initialize();   // Open WLAN handle + find first adapter
    void Shutdown();     // Release WLAN handle

    // Returns cached scan results and triggers a background rescan.
    std::vector<WifiNetwork> Scan();

    // Connect using WPA2-PSK profile. Pass empty password for open networks.
    bool Connect(const std::wstring& ssid, const std::wstring& password = L"");

    // Disconnect the current interface.
    bool Disconnect();

    // Returns SSID of the currently connected network, or empty string.
    std::wstring GetConnectedSsid() const;

private:
    bool         FindInterface();
    std::wstring BuildProfile(const std::wstring& ssid,
                              const std::wstring& password,
                              bool isOpen) const;

    HANDLE m_hClient     = nullptr;
    GUID   m_ifaceGuid   = {};
    bool   m_initialized = false;
};
