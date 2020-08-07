#pragma once

#include <sdkddkver.h>

#if defined( DX12)||defined( DX12_1)
#if (WDK_NTDDI_VERSION) > (NTDDI_WIN10_19H1)
#define DX12UTIMATE
#endif
#endif

#include <d3d12.h>
#include <dxgi1_6.h>

#ifdef DX12UTIMATE
#include "d3dx12Utimate.h"
#else
#include "d3dx12.h"
#endif

#pragma warning(push)
#pragma warning(disable:4324)
#include <windows.h>
#ifdef DX11
#include <D3Dcompiler.h>
#else
#include "dxc/dxcapi.h"
#endif
#include <wrl.h>
#include <shellapi.h>
#pragma warning(pop)

#include <atlbase.h>
using Microsoft::WRL::ComPtr;
using ATL::CComPtr;

#include "../BearGraphics/BearRenderBase.h"

#define R_CHK(a) BEAR_VERIFY(SUCCEEDED(a))

enum EDX12Query
{
	DX12Q_None=0,
	DX12Q_ShaderResource,
	DX12Q_UnorderedAccess,
	DX12Q_Pipeline,
	DX12Q_RayTracingPipeline,
};


#include "DX12AllocatorHeap.h"
#include "DX12Factory.h"
extern DX12Factory* Factory;
#include "DX12Viewport.h"
#include "DX12Context.h"
#include "DX12Shader.h"
#include "DX12IndexBuffer.h"
#include "DX12VertexBuffer.h"

#include "DX12UniformBuffer.h"
#include "DX12RootSignature.h"
#include "DX12Pipeline.h"
#include "DX12PipelineGraphics.h"
#include "DX12PipelineMesh.h"
#include "DX12PipelineRayTracing.h"
#include "DX12DescriptorHeap.h"

#include "DX12ShaderResource.h"
#include "DX12UnorderedAccess.h"
#include "DX12Texture2D.h"
#include "DX12Sampler.h"
#include "DX12Stats.h"

#include "DX12RenderPass.h"
#include "DX12FrameBuffer.h"

#include "DX12TextureCube.h"
#include "DX12StructuredBuffer.h"

#include "DX12BottomLevel.h"
#include "DX12TopLevel.h"
#include "DX12RayTracingShaderTable.h"