// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

//
// HelloHCS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <winerror.h>
#include <wil\resource.h>

// HCS API header file
#include <computecore.h>
#include <computedefs.h>
#include <computenetwork.h>
#include <computestorage.h>

#pragma comment(lib, "computecore.lib")

int main()
{

//
// Helper RAII objects around HCS system handle and HCS operation handle
// HCS_OPERATION handle closed by HcsCloseOperation
// HCS_SYSTEM handle closed by HcsCloseComputeSystem
//
    using unique_hcs_operation = wil::unique_any<HCS_OPERATION, decltype(&HcsCloseOperation), HcsCloseOperation>;
    using unique_hcs_system = wil::unique_any<HCS_SYSTEM, decltype(&HcsCloseComputeSystem), HcsCloseComputeSystem>;

    //
    // Create a virtual machine
    //
    static constexpr wchar_t c_VmConfiguration[] = LR"(
    {
        "SchemaVersion": {
            "Major": 2,
            "Minor": 1
        },
        "Owner": "Sample",
        "ShouldTerminateOnLastHandleClosed": true,
        "VirtualMachine": {
            "Chipset": {
                "Uefi": {
                    "BootThis": {
                        "DevicePath": "Primary disk",
                        "DiskNumber": 0,
                        "DeviceType": "ScsiDrive"
                    }
                }
            },
            "ComputeTopology": {
                "Memory": {
                    "Backing": "Virtual",
                    "SizeInMB": 2048
                },
                "Processor": {
                    "Count": 2
                }
            },
            "Devices": {
                "Scsi": {
                    "Primary disk": {
                        "Attachments": {
                            "0": {
                                "Type": "VirtualDisk",
                                "Path": "c:\\utilityvm.vhdx"
                            }
                        }
                    }
                }
            }
        }
    })";

    // After setting up the JSON document, we need to call into the HCS to create
    // the compute system, in this case, an HCS VM.
    // This operation doesn't need callback
    unique_hcs_operation operation(HcsCreateOperation(nullptr, nullptr));
    unique_hcs_system system;
    THROW_IF_FAILED(HcsCreateComputeSystem(
        L"Sample", // Unique Id
        c_VmConfiguration,
        operation.get(),
        nullptr, // This parameter is not supported yet, always pass NULL
        &system));

    // We need to wait on the operation explicitly because no callback was setup
    // Result document from waiting on the operation would usually contain
    // a JSON blob with either error attribution or operation specific result data
    // (for example, compute system properties if the operation was used to call
    // ::HcsGetComputeSystemProperties).
    // Result document isn't necessarily set all the time, so make sure to check for that!
    wil::unique_hlocal_string resultDoc;
    THROW_IF_FAILED_MSG(HcsWaitForOperationResult(operation.get(), INFINITE, &resultDoc),
        "ResultDoc: %ws", resultDoc.get());
    // For a successful HcsCreateComputeSystem, result document doesn't have anything.

    //
    // Now that the HCS VM is created, we need to start it!
    //
    THROW_IF_FAILED(HcsStartComputeSystem(
        system.get(),
        operation.get(),
        nullptr)); // This parameter is not currently supported, pass NULL.
    THROW_IF_FAILED_MSG(HcsWaitForOperationResult(operation.get(), INFINITE, &resultDoc),
        "ResultDoc: %ws", resultDoc.get());
    wprintf(L"VM started successfully\n");

    //
    // Get the VM memory property and print the result out
    //
    static constexpr wchar_t c_VmQuery[] = LR"(
    {
        "PropertyTypes":[
            "Memory"
        ]
    })";

    THROW_IF_FAILED(HcsGetComputeSystemProperties(
        system.get(),
        operation.get(),
        c_VmQuery));
    THROW_IF_FAILED_MSG(HcsWaitForOperationResult(operation.get(), INFINITE, &resultDoc),
        "ResultDoc: %ws", resultDoc.get());

    // It's a failure if the result document doesn't have anything for
    // ::HcsGetComputeSystemProperties!
    THROW_HR_IF(E_UNEXPECTED, !resultDoc);
    wprintf(L"HCS VM properties:\n%s\n", resultDoc.get());

    // 
    // Enter the logic that should be run before the HCS VM gets terminated.
    //

    //
    // Finally, shut down the virtual machine. Because the sample virtual machine is not created with guest
    // compute service, HcsShutDownComputeSystem is not able to use here. Let's use HcsTerminateComputeSystem
    //
    THROW_IF_FAILED(HcsTerminateComputeSystem(system.get(), operation.get(), nullptr));
    THROW_IF_FAILED_MSG(HcsWaitForOperationResult(operation.get(), INFINITE, &resultDoc),
        "ResultDoc: %ws", resultDoc.get());

    wprintf(L"The operation succeeded\n");
}
