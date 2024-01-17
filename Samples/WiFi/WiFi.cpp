// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

//
// WiFi.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <wlanapi.h>
#include <wil/resource.h>
#include <wil/result.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#pragma comment(lib, "Wlanapi.lib")

std::wstring ConvertToWideString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

class WlanHandle {
    wil::unique_handle handle;

public:
    WlanHandle() noexcept : handle(INVALID_HANDLE_VALUE) {}

    void Open() {
        DWORD dwMaxClient = 2, dwCurVersion = 0;
        HANDLE tempHandle = INVALID_HANDLE_VALUE;
        DWORD dwResult = ::WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &tempHandle);
        if (dwResult != ERROR_SUCCESS) {
            THROW_HR(HRESULT_FROM_WIN32(dwResult));
        }
        handle.reset(tempHandle);
    }

    ~WlanHandle() {
        if (handle) {
            ::WlanCloseHandle(handle.get(), NULL);
        }
    }

    HANDLE get() const { return handle.get(); }
};

void FreeWlanList(WLAN_AVAILABLE_NETWORK_LIST* list) {
    if (list) {
        ::WlanFreeMemory(list);
    }
}

void ScanForWiFi(const WlanHandle& wlanHandle, const GUID* pInterfaceGuid) {
    DWORD dwResult;
    wil::unique_any<WLAN_AVAILABLE_NETWORK_LIST*, decltype(&FreeWlanList), &FreeWlanList> pBssList = nullptr;

    dwResult = ::WlanScan(wlanHandle.get(), pInterfaceGuid, NULL, NULL, NULL);
    if (dwResult != ERROR_SUCCESS) {
        THROW_HR(HRESULT_FROM_WIN32(dwResult));
    }

    dwResult = ::WlanGetAvailableNetworkList(wlanHandle.get(), pInterfaceGuid, 0, NULL, &pBssList);
    if (dwResult != ERROR_SUCCESS) {
        THROW_HR(HRESULT_FROM_WIN32(dwResult));
    }

    if (auto* pList = pBssList.get()) {
        for (DWORD i = 0; i < pList->dwNumberOfItems; i++) {
            WLAN_AVAILABLE_NETWORK& network = pList->Network[i];
            std::string ssidStr(reinterpret_cast<const char*>(network.dot11Ssid.ucSSID), network.dot11Ssid.uSSIDLength);
            std::wstring ssidWStr = ConvertToWideString(ssidStr);

            std::wcout << L"Network " << i + 1 << L": " << ssidWStr << std::endl;
            std::wcout << L"  SSID: " << network.dot11Ssid.ucSSID << std::endl;
            std::wcout << L"  Signal Quality: " << network.wlanSignalQuality << std::endl;
            std::wcout << L"  Authentication: " << network.dot11DefaultAuthAlgorithm << std::endl;
            std::wcout << L"  Encryption: " << network.dot11DefaultCipherAlgorithm << std::endl;
            std::wcout << std::endl;
        }
    }
}

void DisconnectWiFi(const WlanHandle& wlanHandle, const GUID* pInterfaceGuid) {
    // Query the current connection state
    WLAN_CONNECTION_ATTRIBUTES* pConnectionAttributes = nullptr;
    DWORD dataSize = 0;
    DWORD dwResult = ::WlanQueryInterface(wlanHandle.get(), pInterfaceGuid, wlan_intf_opcode_current_connection, NULL, &dataSize, reinterpret_cast<void**>(&pConnectionAttributes), NULL);
    if (dwResult != ERROR_SUCCESS) {
        if (dwResult == ERROR_INVALID_STATE) {
            std::wcout << L"No Wi-Fi network is currently connected or interface not in a state to disconnect." << std::endl;
        }
        else {
            THROW_HR(HRESULT_FROM_WIN32(dwResult));
        }
        return;
    }

    if (pConnectionAttributes && pConnectionAttributes->isState == wlan_interface_state_connected) {
        // Free the memory allocated by WlanQueryInterface
        ::WlanFreeMemory(pConnectionAttributes);

        // Proceed to disconnect
        dwResult = ::WlanDisconnect(wlanHandle.get(), pInterfaceGuid, NULL);
        if (dwResult != ERROR_SUCCESS) {
            THROW_HR(HRESULT_FROM_WIN32(dwResult));
        }
        std::wcout << L"Disconnected from the Wi-Fi network." << std::endl;
    }
    else {
        std::wcout << L"No Wi-Fi network connected." << std::endl;
        if (pConnectionAttributes) {
            ::WlanFreeMemory(pConnectionAttributes);
        }
    }
}

void ConnectToWiFi(const WlanHandle& wlanHandle, const GUID* pInterfaceGuid, const std::wstring& ssid) {
    std::wstring profileXmlPath;
    std::wifstream profileFileStream;
    std::wstringstream profileStream;
    DWORD dwReason;
    DWORD dwResult;

    std::wcout << L"Enter the path to the profile XML file for " << ssid << L": ";
    std::getline(std::wcin, profileXmlPath);

    if (!profileXmlPath.empty() && profileXmlPath.front() == L'"' && profileXmlPath.back() == L'"') {
        profileXmlPath = profileXmlPath.substr(1, profileXmlPath.length() - 2);
    }

    profileFileStream.open(profileXmlPath);
    if (!profileFileStream.is_open()) {
        THROW_HR(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    }

    profileStream << profileFileStream.rdbuf();
    std::wstring profileXml = profileStream.str();

    dwResult = ::WlanSetProfile(wlanHandle.get(), pInterfaceGuid, 0, profileXml.c_str(), NULL, TRUE, NULL, &dwReason);
    if (dwResult != ERROR_SUCCESS) {
        THROW_HR(HRESULT_FROM_WIN32(dwResult));
    }

    WLAN_CONNECTION_PARAMETERS params = {};
    params.wlanConnectionMode = wlan_connection_mode_profile;
    params.strProfile = ssid.c_str();
    params.pDot11Ssid = NULL;
    params.pDesiredBssidList = NULL;
    params.dot11BssType = dot11_BSS_type_any;
    params.dwFlags = 0;

    dwResult = ::WlanConnect(wlanHandle.get(), pInterfaceGuid, &params, NULL);
    if (dwResult != ERROR_SUCCESS) {
        THROW_HR(HRESULT_FROM_WIN32(dwResult));
    }
    std::wcout << L"Initiated connection to: " << ssid << std::endl;
}

void FreeWlanInterfaceInfoList(WLAN_INTERFACE_INFO_LIST* list) {
    if (list) {
        ::WlanFreeMemory(list);
    }
}

enum class Command {
    Scan,
    Connect,
    Disconnect
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::wcerr << "Commands: scan, connect [profile_name], disconnect" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }

    Command parsedCommand;
    std::string command(argv[1]);
    if (command == "scan") {
        parsedCommand = Command::Scan;
    }
    else if (command == "connect" && argc >= 3) {
        parsedCommand = Command::Connect;
    }
    else if (command == "disconnect") {
        parsedCommand = Command::Disconnect;
    }
    else {
        std::wcerr << "Invalid command. Supported commands are: scan, connect [profile_name], disconnect" << std::endl;
        return ERROR_BAD_COMMAND;
    }

    try {
        WlanHandle wlanHandle;
        wlanHandle.Open();
        std::wstring ssid;

        if (parsedCommand == Command::Connect) {
            ssid = ConvertToWideString(argv[2]);
        }

        if (parsedCommand == Command::Scan || parsedCommand == Command::Connect || parsedCommand == Command::Disconnect) {
            // Define a unique smart pointer type for WLAN_INTERFACE_INFO_LIST
            using unique_wlan_interface_info_list = wil::unique_any<WLAN_INTERFACE_INFO_LIST*, decltype(&FreeWlanInterfaceInfoList), FreeWlanInterfaceInfoList>;
            unique_wlan_interface_info_list pIfList = nullptr;

            DWORD dwResult = ::WlanEnumInterfaces(wlanHandle.get(), NULL, &pIfList);
            if (dwResult != ERROR_SUCCESS) {
                THROW_HR(HRESULT_FROM_WIN32(dwResult));
            }

            // Ensure there are interfaces available
            if (!pIfList) {
                std::wcout << L"No Wi-Fi interfaces found." << std::endl;
                return ERROR_NO_MORE_ITEMS;
            }

            const GUID* pInterfaceGuid = &pIfList.get()->InterfaceInfo[0].InterfaceGuid;

            if (parsedCommand == Command::Scan) {
                ScanForWiFi(wlanHandle, pInterfaceGuid);
            }
            else if (parsedCommand == Command::Connect && !ssid.empty()) {
                ConnectToWiFi(wlanHandle, pInterfaceGuid, ssid);
            }
            else if (parsedCommand == Command::Disconnect) {
                DisconnectWiFi(wlanHandle, pInterfaceGuid);
            }
        }
    }
    catch (const wil::ResultException& e) {
        std::wcerr << "An unexpected error occurred.: " << e.what() << std::endl;
        return ERROR_INTERNAL_ERROR;
    }
    catch (const std::exception& e) {
        std::wcerr << "An unexpected error occurred.: " << e.what() << std::endl;
        return ERROR_INTERNAL_ERROR;
    }

    return ERROR_SUCCESS;
}
