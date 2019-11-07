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
#include "BearRenderBase.h"

using Microsoft::WRL::ComPtr;
#define R_CHK(a) BEAR_RASSERT(SUCCEEDED(a))

using namespace BearCore;
#include "DX12RenderInterface.h"
#include "DX12RenderViewport.h"

#include "DX12RenderIndexBuffer.h"
#include "DX12RenderVertexBuffer.h"
#include "DX12RenderUniformBuffer.h"
#include "DX12RenderRootSignature.h"
#include "DX12RenderDescriptorHeap.h"
#include "DX12RenderSamplerState.h"
#include "DX12RenderTexture2D.h"
#include "DX12RenderTexture2DUAV.h"
#include "DX12RenderTargetView.h"
#include "DX12RenderFrameBuffer.h"

#include "DX12RenderPipeline.h"
#include "DX12RenderShader.h"


#include "DX12RenderContext.h"
#include "DX12RenderFactory.h"


extern DX12RenderFactory *Factory;