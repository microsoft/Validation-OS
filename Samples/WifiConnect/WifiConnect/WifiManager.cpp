#include "WifiManager.h"
#include <unordered_map>
#pragma comment(lib, "wlanapi.lib")

// ── Lifecycle ────────────────────────────────────────────────────────────────

bool WifiManager::Initialize()
{
    DWORD negotiatedVersion = 0;
    if (WlanOpenHandle(2, nullptr, &negotiatedVersion, &m_hClient) != ERROR_SUCCESS)
        return false;

    m_initialized = FindInterface();
    return m_initialized;
}

void WifiManager::Shutdown()
{
    if (m_hClient)
    {
        WlanCloseHandle(m_hClient, nullptr);
        m_hClient     = nullptr;
        m_initialized = false;
    }
}

// ── Private helpers ───────────────────────────────────────────────────────────

bool WifiManager::FindInterface()
{
    PWLAN_INTERFACE_INFO_LIST pList = nullptr;
    if (WlanEnumInterfaces(m_hClient, nullptr, &pList) != ERROR_SUCCESS)
        return false;

    const bool found = (pList->dwNumberOfItems > 0);
    if (found)
        m_ifaceGuid = pList->InterfaceInfo[0].InterfaceGuid;

    WlanFreeMemory(pList);
    return found;
}

std::wstring WifiManager::BuildProfile(const std::wstring& ssid,
                                       const std::wstring& password,
                                       bool isOpen) const
{
    const UINT resId = isOpen ? IDR_WIFI_PROFILE_OPEN : IDR_WIFI_PROFILE_WPA2;
    std::wstring tmpl = Utils::LoadResource(resId);

    tmpl = Utils::Replace(tmpl, L"{SSID}", Utils::XmlEscape(ssid));
    if (!isOpen)
        tmpl = Utils::Replace(tmpl, L"{PASSWORD}", Utils::XmlEscape(password));

    return tmpl;
}

// ── Public API ────────────────────────────────────────────────────────────────

std::vector<WifiNetwork> WifiManager::Scan()
{
    std::vector<WifiNetwork> networks;
    if (!m_initialized)
        return networks;

    // Kick off a background hardware scan; results arrive within ~4 s.
    // We return the currently cached list immediately so the UI is responsive.
    WlanScan(m_hClient, &m_ifaceGuid, nullptr, nullptr, nullptr);

    PWLAN_AVAILABLE_NETWORK_LIST pList = nullptr;
    if (WlanGetAvailableNetworkList(m_hClient, &m_ifaceGuid,
                                    WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES,
                                    nullptr, &pList) != ERROR_SUCCESS)
        return networks;

    std::unordered_map<std::wstring, WifiNetwork> seen;

    for (DWORD i = 0; i < pList->dwNumberOfItems; ++i)
    {
        const auto& n = pList->Network[i];
        if (n.dot11Ssid.uSSIDLength == 0)
            continue;   // skip hidden networks

        WifiNetwork net;
        net.ssid          = Utils::SsidToWString(n.dot11Ssid);
        net.signalQuality = n.wlanSignalQuality;
        net.isConnected   = (n.dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED) != 0;
        net.isSecured     = (n.dot11DefaultAuthAlgorithm != DOT11_AUTH_ALGO_80211_OPEN);

        // Keep the entry with the highest signal; merge isConnected across duplicates.
        auto it = seen.find(net.ssid);
        if (it == seen.end())
        {
            seen.emplace(net.ssid, std::move(net));
        }
        else
        {
            if (net.signalQuality > it->second.signalQuality)
                it->second.signalQuality = net.signalQuality;
            if (net.isConnected)
                it->second.isConnected = true;
        }
    }

    networks.reserve(seen.size());
    for (auto& kv : seen)
        networks.push_back(std::move(kv.second));

    WlanFreeMemory(pList);
    return networks;
}

bool WifiManager::Connect(const std::wstring& ssid, const std::wstring& password)
{
    if (!m_initialized)
        return false;

    const bool isOpen      = password.empty();
    const std::wstring xml = BuildProfile(ssid, password, isOpen);

    // Store the profile so Windows can reconnect automatically later.
    DWORD reasonCode = 0;
    if (WlanSetProfile(m_hClient, &m_ifaceGuid, 0,
                       xml.c_str(), nullptr, TRUE, nullptr, &reasonCode) != ERROR_SUCCESS)
        return false;

    WLAN_CONNECTION_PARAMETERS params{};
    params.wlanConnectionMode = wlan_connection_mode_profile;
    params.strProfile         = ssid.c_str();   // profile name == SSID
    params.dot11BssType       = dot11_BSS_type_infrastructure;

    return WlanConnect(m_hClient, &m_ifaceGuid, &params, nullptr) == ERROR_SUCCESS;
    // Note: WlanConnect returns immediately; the connection completes asynchronously.
}

bool WifiManager::Disconnect()
{
    if (!m_initialized)
        return false;

    return WlanDisconnect(m_hClient, &m_ifaceGuid, nullptr) == ERROR_SUCCESS;
}

std::wstring WifiManager::GetConnectedSsid() const
{
    if (!m_initialized)
        return L"";

    PWLAN_CONNECTION_ATTRIBUTES pAttr = nullptr;
    DWORD dataSize                    = sizeof(WLAN_CONNECTION_ATTRIBUTES);
    WLAN_OPCODE_VALUE_TYPE opCode{};

    const DWORD result = WlanQueryInterface(
        m_hClient, &m_ifaceGuid,
        wlan_intf_opcode_current_connection,
        nullptr, &dataSize,
        reinterpret_cast<PVOID*>(&pAttr), &opCode);

    if (result != ERROR_SUCCESS)
        return L"";

    std::wstring ssid;
    if (pAttr->isState == wlan_interface_state_connected)
        ssid = Utils::SsidToWString(pAttr->wlanAssociationAttributes.dot11Ssid);

    WlanFreeMemory(pAttr);
    return ssid;
}
