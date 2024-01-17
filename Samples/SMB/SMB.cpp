// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

//
// SMB.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <lm.h>
#include <iostream>
#include <iomanip>
#include <winerror.h>
#include <string>

#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Advapi32.lib")

int wmain(int argc, TCHAR* lpszArgv[])
{
    PSHARE_INFO_502 BufPtr, p;
    NET_API_STATUS res;
    std::wstring lpszServer;
    DWORD er = 0, tr = 0, resume = 0, i;

    switch (argc)
    {
    case 2:
        lpszServer = lpszArgv[1];
        break;
    default:
        std::cerr << "Usage: smb.exe <servername>\n";
        return ERROR_INVALID_PARAMETER;
    }
    //
    // Print a report header.
    //
    std::cout << "Share:              Local Path:                   Uses:   Descriptor:\n";
    std::cout << "---------------------------------------------------------------------\n";
    //
    // Call the NetShareEnum function; specify level 502.
    //
    do // begin do
    {
        res = ::NetShareEnum((LPWSTR)lpszServer.c_str(), 502, (LPBYTE*)&BufPtr, MAX_PREFERRED_LENGTH, &er, &tr, &resume);
        //
        // If the call succeeds,
        //
        if (res == ERROR_SUCCESS || res == ERROR_MORE_DATA)
        {
            p = BufPtr;
            //
            // Loop through the entries;
            //  print retrieved data.
            //
            for (i = 1; i <= er; i++)
            {
                std::wcout << std::left
                    << std::setw(20) << p->shi502_netname
                    << std::setw(30) << p->shi502_path
                    << std::setw(8) << p->shi502_current_uses;

                //
                // Validate the value of the 
                //  shi502_security_descriptor member.
                //
                if (::IsValidSecurityDescriptor(p->shi502_security_descriptor))
                    std::cout << "Yes\n";
                else
                    std::cout << "No\n";
                p++;
            }
            //
            // Free the allocated buffer.
            //
            ::NetApiBufferFree(BufPtr);
        }
        else
        {
            std::cerr << "Error: " << res << std::endl;
            return res;
        }
    }
    // Continue to call NetShareEnum while 
    //  there are more entries. 
    // 
    while (res == ERROR_MORE_DATA); // end do
    return ERROR_SUCCESS;
}
