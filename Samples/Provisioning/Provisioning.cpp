// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

//
// Provisioning.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <wil/com.h>
#include <winerror.h>

#pragma comment(lib, "wbemuuid.lib")

BOOL wmi_run();
BOOL wmi_getDriveLetters();

wil::com_ptr<IWbemLocator> pLoc;
wil::com_ptr<IWbemServices> pSvc;

int main(int argc, char** argv)
{
    // Initialize COM
    auto CoInit = wil::CoInitializeEx(COINIT_MULTITHREADED);
    wmi_run();
    wmi_getDriveLetters();

    // Explicitly clear COM pointers before uninitializing COM
    pSvc.reset();
    pLoc.reset();

    CoUninitialize();

    return 0;
}

//
// Step 1-4 at:
// https://msdn.microsoft.com/library/aa390423(VS.85).aspx
BOOL wmi_run()
{

    // Step 1: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------

    HRESULT hres = ::CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
    );

    if (FAILED(hres))
    {
        std::wcerr << "Failed to initialize security. Error code = 0x"
            << std::hex << hres << std::endl;
        return ERROR_INTERNAL_ERROR; // Program has failed.
    }

    // Step 2: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    hres = ::CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);


    if (FAILED(hres))
    {
        std::wcerr << "Failed to create IWbemLocator object."
            << " Err code = 0x"
            << std::hex << hres << std::endl;
        return ERROR_INTERNAL_ERROR; // Program has failed.
    }

    // Step 3: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (e.g. Kerberos)
        0,                       // Context object
        &pSvc                    // pointer to IWbemServices proxy
    );

    if (FAILED(hres))
    {
        std::wcerr << "Could not connect. Error code = 0x"
            << std::hex << hres << std::endl;
        return ERROR_INTERNAL_ERROR; // Program has failed.
    }

    std::wcout << "Connected to ROOT\\CIMV2 WMI namespace" << std::endl;

    // Step 4: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = ::CoSetProxyBlanket(
        pSvc.get(),                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(hres))
    {
        std::wcerr << "Could not set proxy blanket. Error code = 0x"
            << std::hex << hres << std::endl;
        return ERROR_INTERNAL_ERROR; // Program has failed.
    }
    return 0;
}

//
// get Drives, logical Drives and Driveletters
BOOL wmi_getDriveLetters()
{

    // Use the IWbemServices pointer to make requests of WMI.
    // Make requests here:
    HRESULT hres;
    wil::com_ptr<IEnumWbemClassObject> pEnumerator;

    // get localdrives
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_DiskDrive"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres)) {
        std::wcerr << "Query for processes failed. "
            << "Error code = 0x"
            << std::hex << hres << std::endl;
        return FALSE; // Program has failed.
    }
    else {
        wil::com_ptr<IWbemClassObject> pclsObj;
        ULONG uReturn = 0;
        while (pEnumerator) {
            pclsObj.reset();
            hres = pEnumerator->Next(WBEM_INFINITE, 1,
                &pclsObj, &uReturn);
            if (0 == uReturn) break;

            wil::unique_variant vtProp;
            hres = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);

            // adjust string
            std::wstring tmp = vtProp.bstrVal;
            tmp = tmp.substr(4);

            std::wstring wstrQuery = L"Associators of {Win32_DiskDrive.DeviceID='\\\\.\\";
            wstrQuery += tmp;
            wstrQuery += L"'} where AssocClass=Win32_DiskDriveToDiskPartition";

            // reference drive to partition
            wil::com_ptr<IEnumWbemClassObject> pEnumerator1;
            hres = pSvc->ExecQuery(
                bstr_t("WQL"),
                bstr_t(wstrQuery.c_str()),
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                pEnumerator1.put());

            if (FAILED(hres)) {
                std::wcerr << "Query for processes failed. "
                    << "Error code = 0x"
                    << std::hex << hres << std::endl;
                return FALSE; // Program has failed.
            }
            else {

                wil::com_ptr<IWbemClassObject> pclsObj1;
                ULONG uReturn1 = 0;
                while (pEnumerator1) {
                    hres = pEnumerator1->Next(WBEM_INFINITE, 1,
                        pclsObj1.put(), &uReturn1);
                    if (0 == uReturn1) break;

                    // reference logical drive to partition
                    wil::unique_variant vtProp1;
                    hres = pclsObj1->Get(L"DeviceID", 0, &vtProp1, 0, 0);
                    std::wstring wstrQuery = L"Associators of {Win32_DiskPartition.DeviceID='";
                    wstrQuery += vtProp1.bstrVal;
                    wstrQuery += L"'} where AssocClass=Win32_LogicalDiskToPartition";

                    wil::com_ptr<IEnumWbemClassObject> pEnumerator2;
                    hres = pSvc->ExecQuery(
                        bstr_t("WQL"),
                        bstr_t(wstrQuery.c_str()),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        pEnumerator2.put());

                    if (FAILED(hres)) {
                        std::wcerr << "Query for processes failed. "
                            << "Error code = 0x"
                            << std::hex << hres << std::endl;
                        return FALSE; // Program has failed.
                    }
                    else {

                        // get driveletter
                        wil::com_ptr<IWbemClassObject> pclsObj2;
                        ULONG uReturn2 = 0;
                        while (pEnumerator2) {
                            hres = pEnumerator2->Next(WBEM_INFINITE, 1,
                                pclsObj2.put(), &uReturn2);
                            if (0 == uReturn2) break;

                            wil::unique_variant vtProp2;
                            hres = pclsObj2->Get(L"DeviceID", 0, &vtProp2, 0, 0);

                            // print result
                            std::wcout << vtProp.bstrVal << L" : " << vtProp2.bstrVal << std::endl;
                        }
                    }
                }
            }
        }
    }
    return TRUE;
}