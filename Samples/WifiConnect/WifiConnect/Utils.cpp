#include "Utils.h"

namespace Utils
{
    // Converts a DOT11_SSID byte array (UTF-8) to a wide string.
    //
    // Example:
    //   DOT11_SSID s = { 7, {'M','y','W','i','f','i','!'} };
    //   SsidToWString(s) → L"MyWifi!"
    std::wstring SsidToWString(const DOT11_SSID& ssid)
    {
        if (ssid.uSSIDLength == 0)
            return L"";

        const char* pBytes  = reinterpret_cast<const char*>(ssid.ucSSID);
        const int   byteLen = static_cast<int>(ssid.uSSIDLength);

        const int wLen = MultiByteToWideChar(CP_UTF8, 0, pBytes, byteLen, nullptr, 0);
        std::wstring result(wLen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, pBytes, byteLen, &result[0], wLen);
        return result;
    }

    // Loads an RCDATA resource embedded in the binary and returns it as a
    // wide string. The resource data is assumed to be UTF-8 encoded.
    //
    // Example:
    //   LoadResource(IDR_WIFI_PROFILE_WPA2) → L"<?xml version=\"1.0\"?>..."
    std::wstring LoadResource(UINT resourceId)
    {
        const HRSRC hRes = FindResourceW(nullptr,
                                         MAKEINTRESOURCEW(resourceId),
                                         RT_RCDATA);
        if (!hRes) return L"";

        const HGLOBAL hData = ::LoadResource(nullptr, hRes);
        if (!hData) return L"";

        const char* pData  = static_cast<const char*>(LockResource(hData));
        const DWORD cbData = SizeofResource(nullptr, hRes);
        if (!pData || cbData == 0) return L"";

        const int wLen = MultiByteToWideChar(CP_UTF8, 0,
                                             pData, static_cast<int>(cbData),
                                             nullptr, 0);
        std::wstring result(wLen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0,
                            pData, static_cast<int>(cbData),
                            &result[0], wLen);
        return result;
    }

    // Replaces every occurrence of 'from' with 'to' inside 'str'.
    // Returns the modified string; the original is unchanged.
    //
    // Example:
    //   Replace(L"Hello {NAME}!", L"{NAME}", L"World") → L"Hello World!"
    std::wstring Replace(std::wstring str,
                         const std::wstring& from,
                         const std::wstring& to)
    {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != std::wstring::npos)
        {
            str.replace(pos, from.size(), to);
            pos += to.size();
        }
        return str;
    }

    // Escapes characters that are special in XML so the value is safe to
    // embed inside an element or attribute.
    //
    // Example:
    //   XmlEscape(L"AT&T <WiFi>") → L"AT&amp;T &lt;WiFi&gt;"
    std::wstring XmlEscape(const std::wstring& s)
    {
        std::wstring out;
        out.reserve(s.size());
        for (const wchar_t c : s)
        {
            switch (c)
            {
            case L'&':  out += L"&amp;";  break;
            case L'<':  out += L"&lt;";   break;
            case L'>':  out += L"&gt;";   break;
            case L'"':  out += L"&quot;"; break;
            case L'\'': out += L"&apos;"; break;
            default:    out += c;         break;
            }
        }
        return out;
    }
}
