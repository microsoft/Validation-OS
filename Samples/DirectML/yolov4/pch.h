//--------------------------------------------------------------------------------------
// pch.h
//
// Header for standard system include files.
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------

#pragma once

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0A00
#include <SDKDDKVer.h>

#define _CRT_RAND_S

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wrl/client.h>
#include <wrl/event.h>

#include <d3d12.h>
#include <d3d11_1.h>
#include <d3d10.h>

#if defined(NTDDI_WIN10_RS2)
#include <dxgi1_6.h>
#else
#include <dxgi1_5.h>
#endif

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wincodec.h>

#include "d3dx12.h"

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>
#include <vector>
#ifdef __cpp_lib_span 
#include <span>
#endif
#include <map>
#include <set>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include <stdio.h>
#include <pix3.h>
#include <variant>
#include <optional>

#include "directxtk12/CommonStates.h"
#include "directxtk12/Effects.h"
#include "directxtk12/GamePad.h"
#include "directxtk12/GraphicsMemory.h"
#include "directxtk12/DescriptorHeap.h"
#include "directxtk12/DirectXHelpers.h"
#include "directxtk12/Keyboard.h"
#include "directxtk12/Mouse.h"
#include "directxtk12/PrimitiveBatch.h"
#include "directxtk12/RenderTargetState.h"
#include "directxtk12/ResourceUploadBatch.h"
#include "directxtk12/SimpleMath.h"
#include "directxtk12/SpriteBatch.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/VertexTypes.h"

#define DML_TARGET_VERSION_USE_LATEST
#include "DirectML.h"
#include "DirectMLX.h"

// Use video frames as input to the DirectML model, instead of a static texture.
#define USE_VIDEO 1

// Let DirectML manage the data in the weight tensors. This can be faster on some hardware.
#define DML_MANAGED_WEIGHTS 1

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        virtual const char* what() const override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}