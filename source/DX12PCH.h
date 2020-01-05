#pragma once
#pragma warning(push)
#pragma warning(disable:4324)
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include "d3d12_1.h"
#include "d3dx12.h"
#include <wrl.h>
#include <shellapi.h>
#ifdef RTX
#include "dxc/dxcapi.h"
#endif
#pragma warning(pop)
#include <atlbase.h>
using Microsoft::WRL::ComPtr;
using ATL::CComPtr;

#include "BearRenderBase.h"

#define R_CHK(a) BEAR_RASSERT(SUCCEEDED(a))

#include "DX12Factory.h"
extern DX12Factory* Factory;
#include "DX12Viewport.h"
#include "DX12Context.h"
#include "DX12Shader.h"
#include "DX12IndexBuffer.h"
#include "DX12VertexBuffer.h"
#include "DX12Pipeline.h"