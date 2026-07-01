#pragma once
#include <windows.h>
#include <wlanapi.h>
#include <string>

namespace Utils
{
    // Converts a DOT11_SSID byte array (UTF-8) to a wide string.
    //
    // Example:
    //   DOT11_SSID s = { 7, {'M','y','W','i','f','i','!'} };
    //   SsidToWString(s) → L"MyWifi!"
    std::wstring SsidToWString(const DOT11_SSID& ssid);

    // Loads an RCDATA resource embedded in the binary and returns it as a
    // wide string. The resource data is assumed to be UTF-8 encoded.
    //
    // Example:
    //   LoadResource(IDR_WIFI_PROFILE_WPA2) → L"<?xml version=\"1.0\"?>..."
    std::wstring LoadResource(UINT resourceId);

    // Replaces every occurrence of 'from' with 'to' inside 'str'.
    // Returns the modified string; the original is unchanged.
    //
    // Example:
    //   Replace(L"Hello {NAME}!", L"{NAME}", L"World") → L"Hello World!"
    std::wstring Replace(std::wstring str,
                         const std::wstring& from,
                         const std::wstring& to);

    // Escapes characters that are special in XML so the value is safe to
    // embed inside an element or attribute.
    //
    // Example:
    //   XmlEscape(L"AT&T <WiFi>") → L"AT&amp;T &lt;WiFi&gt;"
    std::wstring XmlEscape(const std::wstring& s);
}
