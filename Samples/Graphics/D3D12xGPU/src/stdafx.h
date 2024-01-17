// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.


// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

//#define PIXSUPPORT  // An override to disable new features that PIX doesn't have a support implemented for yet.
#ifdef PIXSUPPORT
#include <dxgi1_4.h>
#else
#define USE_DXGI_1_6
#include <dxgi1_6.h>
#endif
#include <d3d12.h>
#include "d3dx12.h"
#include <d2d1_3.h>
#include <D3Dcompiler.h>
#include <dwrite.h>
#include <d3d11on12.h>
#include <DirectXMath.h>
#include <pix3.h>

#include <process.h>
#include <float.h>
#include <cmath>
#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
