#include "DX12PCH.h"
bool GDebugRender;  
static bool GExperimental;
static const GUID D3D12ExperimentalShaderModelsID =
{ /* 76f5573e-f13a-40f5-b297-81ce9e18933f */
	0x76f5573e,
	0xf13a,
	0x40f5,
	{ 0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f }
};
DX12Factory::DX12Factory() :bSupportMeshShader(false), bSupportRayTracing(false)
{
	UINT dxgiFactoryFlags = 0;
	m_Default_FenceEvent = 0;
	

	GDebugRender = BearString::Find(GetCommandLine(), TEXT("-debugrender"));
	if(!GDebugRender)
		GDebugRender = BearString::Find(GetCommandLine(), TEXT("-drender"));
#if defined(_DEBUG)
	GDebugRender = true;
#endif

	GExperimental = BearString::Find(GetCommandLine(), TEXT("-debugexperimental"));
	if (!GExperimental)
		GExperimental = BearString::Find(GetCommandLine(), TEXT("-dexperimental"));




#ifdef DX12UTIMATE
	if(GExperimental)
		D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModelsID, nullptr, nullptr);
#endif 

	if(GDebugRender)

	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}

	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&GIFactory))))
	{
		return;
	}
	ComPtr<IDXGIAdapter1> hardwareAdapter;

	D3D_FEATURE_LEVEL Level;

	GetHardwareAdapter(GIFactory.Get(), &hardwareAdapter, Level);
	if (hardwareAdapter.Get() == 0)
		return;
	IDXGIOutput *Output;
	hardwareAdapter->EnumOutputs(0, &Output);
	{
		UINT count = 0;

		Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, 0);
		m_GIVideoMode.resize(count);
		Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, &m_GIVideoMode[0]);
	}

	Output->Release();
	if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), Level, IID_PPV_ARGS(&Device))))
	{
		return;
	}

#ifdef RTX
	if (FAILED(Device->QueryInterface(IID_PPV_ARGS(&RTXDevice))))
	{
		return;
	}
#endif
	if (GDebugRender)
	{

		ComPtr<ID3D12InfoQueue> InfoQueue;
		if (SUCCEEDED(Device.As(&InfoQueue)))
		{
			InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			D3D12_MESSAGE_SEVERITY Severities[] =
				{
					D3D12_MESSAGE_SEVERITY_INFO};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,						  // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,					  // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;

			R_CHK(InfoQueue->PushStorageFilter(&NewFilter));
		}

	}
	CbvSrvUavDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	SamplerDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	{
#ifndef DX11
		R_CHK(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DxcCompiler)));
		R_CHK(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&DxcLibrary)));
		R_CHK(DxcLibrary->CreateIncludeHandler(&DxcIncludeHandler));
#endif
	}
	{
		m_Default_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_Default_FenceEvent == nullptr)
		{
			R_CHK(HRESULT_FROM_WIN32(GetLastError()));
		}
		R_CHK(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Default_CommandAllocator)));
		R_CHK(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_Default_CommandAllocator.Get(), 0, IID_PPV_ARGS(&CommandList)));
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			R_CHK(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Default_CommandQueue)));
		}
		R_CHK(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Default_Fence)));
		m_Default_FenceValue = 1;
	}
	{
		CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc;
		RootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		R_CHK(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		R_CHK(Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
	}
}

DX12Factory::~DX12Factory()
{
#ifndef DX11
	{
		if(DxcIncludeHandler)
		DxcIncludeHandler->Release();
	}
#endif
	{
		ViewHeapAllocator.clear();
		SamplersHeapAllocator.clear();
	}
	{
		if(CommandList)
		R_CHK(CommandList->Close());

		if (m_Default_FenceEvent)
			CloseHandle(m_Default_FenceEvent);
		m_Default_FenceEvent = 0;
	}
}
BearRHI::BearRHIViewport *DX12Factory::CreateViewport(void* Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearViewportDescription&Description)
{
	return bear_new<DX12Viewport>(Handle, Width,Height,Fullscreen,VSync,Description);
}

BearRHI::BearRHIContext *DX12Factory::CreateContext()
{
	return bear_new<DX12Context>();
}

BearRHI::BearRHIShader* DX12Factory::CreateShader(BearShaderType Type)
{
	return bear_new< DX12Shader>(Type);
}

BearRHI::BearRHIVertexBuffer* DX12Factory::CreateVertexBuffer()
{
	return bear_new<DX12VertexBuffer>();
}

BearRHI::BearRHIIndexBuffer* DX12Factory::CreateIndexBuffer()
{
	return bear_new<DX12IndexBuffer>();
}

BearRHI::BearRHIUniformBuffer* DX12Factory::CreateUniformBuffer(bsize Stride, bsize Count, bool Dynamic)
{
	return bear_new<DX12UniformBuffer>(Stride, Count, Dynamic);
}

BearRHI::BearRHIRootSignature* DX12Factory::CreateRootSignature(const BearRootSignatureDescription& Description)
{
	return bear_new< DX12RootSignature>(Description);
}

BearRHI::BearRHIDescriptorHeap* DX12Factory::CreateDescriptorHeap(const BearDescriptorHeapDescription& Description)
{
	return  bear_new<DX12DescriptorHeap>(Description);;
}

BearRHI::BearRHIPipelineGraphics* DX12Factory::CreatePipelineGraphics(const BearPipelineGraphicsDescription& Description)
{
	return bear_new<DX12PipelineGraphics>(Description);
}

BearRHI::BearRHIPipelineMesh* DX12Factory::CreatePipelineMesh(const BearPipelineMeshDescription& Description)
{
	return bear_new<DX12PipelineMesh>(Description);
}

BearRHI::BearRHIPipelineRayTracing* DX12Factory::CreatePipelineRayTracing(const BearPipelineRayTracingDescription& Description)
{
	return bear_new<DX12PipelineRayTracing>(Description);;
}

BearRHI::BearRHITexture2D* DX12Factory::CreateTexture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, BearTextureUsage TypeUsage, void* data, bool UAV)
{
	return  bear_new<DX12Texture2D>(Width,Height,Mips,Count,PixelFormat, TypeUsage,data, UAV);;
}

BearRHI::BearRHITextureCube* DX12Factory::CreateTextureCube(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, BearTextureUsage TypeUsage, void* data)
{
	return  bear_new<DX12TextureCube>(Width, Height, Mips, Count, PixelFormat, TypeUsage, data);;
}

BearRHI::BearRHIStructuredBuffer* DX12Factory::CreateStructuredBuffer(bsize size, void* data,bool UAV)
{
	return  bear_new<DX12StructuredBuffer>(size, data, UAV);
}

BearRHI::BearRHITexture2D* DX12Factory::CreateTexture2D(bsize Width, bsize Height, BearRenderTargetFormat Format)
{
	return bear_new<DX12Texture2D>(Width, Height, Format);
}

BearRHI::BearRHITexture2D* DX12Factory::CreateTexture2D(bsize Width, bsize Height, BearDepthStencilFormat Format)
{
	return bear_new<DX12Texture2D>(Width, Height, Format);
}

BearRHI::BearRHIRenderPass *DX12Factory::CreateRenderPass(const BearRenderPassDescription& Description)
{
	return bear_new<DX12RenderPass>(Description);
}

BearRHI::BearRHIFrameBuffer* DX12Factory::CreateFrameBuffer(const BearFrameBufferDescription& Description)
{
	return bear_new<DX12FrameBuffer>(Description);
}

BearRHI::BearRHISampler* DX12Factory::CreateSampler(const BearSamplerDescription& Description)
{
	return  bear_new<DX12SamplerState>(Description);
}

BearRHI::BearRHIBottomLevel* DX12Factory::CreateBottomLevel(const BearBottomLevelDescription& Description)
{
	return  bear_new<DX12BottomLevel>(Description);
}

BearRHI::BearRHITopLevel* DX12Factory::CreateTopLevel(const BearTopLevelDescription& Description)
{
	return  bear_new<DX12TopLevel>(Description);
}

DXGI_FORMAT DX12Factory::Translation(BearTexturePixelFormat format)
{
	switch (format)
	{
	case TPF_R8:
		return DXGI_FORMAT_R8_UNORM;
		break;
	case TPF_R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
		break;
	case TPF_R8G8B8:
		BEAR_ASSERT(!"not support R8G8B8");
		break;
	case TPF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case TPF_R32F:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case TPF_R32G32F:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case TPF_R32G32B32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case TPF_R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case TPF_BC1:
	case TPF_BC1a:
		return DXGI_FORMAT_BC1_UNORM;
	case TPF_BC2:
		return DXGI_FORMAT_BC2_UNORM;
	case TPF_BC3:
		return DXGI_FORMAT_BC3_UNORM;
	case TPF_BC4:
		return DXGI_FORMAT_BC4_UNORM;
	case TPF_BC5:
		return DXGI_FORMAT_BC5_UNORM;
	case TPF_BC6:
		return DXGI_FORMAT_BC6H_UF16;
	case TPF_BC7:
		return DXGI_FORMAT_BC7_UNORM;
	default:
		BEAR_CHECK(0);;
	}
	return DXGI_FORMAT_UNKNOWN;

}

D3D12_TEXTURE_ADDRESS_MODE DX12Factory::Translation(BearSamplerAddressMode format)
{
	switch (format)
	{
	case SAM_WRAP:
		return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		break;
	case SAM_MIRROR:
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		break;
	case SAM_CLAMP:
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		break;
	case SAM_BORDER:
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		break;
	default:
		BEAR_CHECK(0);;;
	}
	return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}


void DX12Factory::GetHardwareAdapter(
#ifdef DX12_1
	IDXGIFactory4* pFactory
#else
	IDXGIFactory2 *pFactory
#endif
	, IDXGIAdapter1 **ppAdapter, D3D_FEATURE_LEVEL &Level)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
#ifdef DX11
		Level = D3D_FEATURE_LEVEL_11_0;
#elif DX12
		Level = D3D_FEATURE_LEVEL_12_0;
#elif DX12_1
		Level = D3D_FEATURE_LEVEL_12_1;
#else
#error "Unkwon"
#endif
		ComPtr<ID3D12Device> LDevice;

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), Level, IID_PPV_ARGS(&LDevice))))
		{

#ifndef DX11
			{
				D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};
				if (SUCCEEDED(LDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData))))
				{
					bSupportRayTracing = featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
				}
			}
#endif
		

#ifndef DX11
			D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
			if (FAILED(LDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) ||
#ifdef DX12
			(shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
#elif DX12_1
			(shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_3))
#endif
			{

				continue;
			}
#ifdef DX12UTIMATE
			else
			{
				bSupportMeshShader = shaderModel.HighestShaderModel == D3D_SHADER_MODEL_6_5;
			}
#endif
#endif

#ifdef DX12UTIMATE
			if (GExperimental)
			{
				D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
				if (FAILED(LDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features))) || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
				{
					bSupportMeshShader = false;
				}
			}
			else
			{
				bSupportMeshShader = false;
			}
#else
			bSupportMeshShader = false;
#endif

			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
void DX12Factory::LockCommandList()
{
	m_Default_CommandMutex.Lock();
}

void DX12Factory::UnlockCommandList(ID3D12CommandQueue *CommandQueue)
{
	R_CHK(CommandList->Close());
	ID3D12CommandList *ppCommandLists[] = {CommandList.Get()};
	if (CommandQueue)
		CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	else
		m_Default_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	const uint64 fence = m_Default_FenceValue;
	if (CommandQueue)
	{
		R_CHK(CommandQueue->Signal(m_Default_Fence.Get(), fence));
	}
	else
	{
		R_CHK(m_Default_CommandQueue->Signal(m_Default_Fence.Get(), fence));
	}
	m_Default_FenceValue++;
	if (m_Default_Fence->GetCompletedValue() < fence)
	{
		R_CHK(m_Default_Fence->SetEventOnCompletion(fence, m_Default_FenceEvent));
		WaitForSingleObject(m_Default_FenceEvent, INFINITE);
	}
	R_CHK(m_Default_CommandAllocator->Reset());
	R_CHK(CommandList->Reset(m_Default_CommandAllocator.Get(), 0));
	m_Default_CommandMutex.Unlock();
}


D3D12_BLEND DX12Factory::Translate(BearBlendFactor format)
{
	switch (format)
	{
	case BF_ZERO:
		return D3D12_BLEND::D3D12_BLEND_ZERO;
		break;
	case BF_ONE:
		return D3D12_BLEND::D3D12_BLEND_ONE;
		break;
	case BF_SRC_COLOR:
		return D3D12_BLEND::D3D12_BLEND_SRC_COLOR;
		break;
	case BF_INV_SRC_COLOR:
		return D3D12_BLEND::D3D12_BLEND_INV_SRC_COLOR;
		break;
	case BF_SRC_ALPHA:
		return D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
		break;
	case BF_INV_SRC_ALPHA:
		return D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case BF_DEST_ALPHA:
		return D3D12_BLEND::D3D12_BLEND_DEST_ALPHA;
		break;
	case BF_INV_DEST_ALPHA:
		return D3D12_BLEND::D3D12_BLEND_INV_DEST_ALPHA;
		break;
	case BF_DEST_COLOR:
		return D3D12_BLEND::D3D12_BLEND_DEST_COLOR;
		break;
	case BF_INV_DEST_COLOR:
		return D3D12_BLEND::D3D12_BLEND_INV_DEST_COLOR;
		break;
	case BF_BLEND_FACTOR:
		return D3D12_BLEND::D3D12_BLEND_BLEND_FACTOR;
		break;
	case BF_INV_BLEND_FACTOR:
		return D3D12_BLEND::D3D12_BLEND_INV_BLEND_FACTOR;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return D3D12_BLEND_ZERO;
}

D3D12_BLEND_OP DX12Factory::Translate(BearBlendOp format)
{
	switch (format)
	{
	case BO_ADD:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
		break;
	case BO_SUBTRACT:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_SUBTRACT;
		break;
	case BO_REV_SUBTRACT:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_REV_SUBTRACT;
		break;
	case BO_MIN:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_MIN;
		break;
	case BO_MAX:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_MAX;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
}

D3D12_COMPARISON_FUNC DX12Factory::Translate(BearCompareFunction format)
{
	switch (format)
	{
	case CF_NEVER:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	case CF_ALWAYS:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	case CF_EQUAL:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;
		break;
	case CF_NOTEQUAL:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NOT_EQUAL;
		break;
	case CF_LESS:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
		break;
	case CF_GREATER:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER;
		break;
	case CF_LESSEQUAL:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	case CF_GREATEREQUAL:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return  D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
}

D3D12_STENCIL_OP DX12Factory::Translate(BearStencilOp format)
{
	switch (format)
	{
	case SO_KEEP:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
		break;
	case SO_ZERO:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_ZERO;
		break;
	case SO_REPLACE:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_REPLACE;
		break;
	case SO_INCR_SAT:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR_SAT;
		break;
	case SO_DECR_SAT:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR_SAT;
		break;
	case SO_INVERT:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INVERT;
		break;
	case SO_INCR:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR;
		break;
	case SO_DECR:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return  D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
}


D3D12_CULL_MODE DX12Factory::Translate(BearRasterizerCullMode format)
{
	switch (format)
	{
	case RCM_FRONT:
		return D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;
		break;
	case RCM_BACK:
		return D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
		break;
	case RCM_NONE:
		return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
}

D3D12_FILL_MODE DX12Factory::Translate(BearRasterizerFillMode format)
{
	switch (format)
	{
	case RFM_WIREFRAME:
		return D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME;
		break;
	case RFM_SOLID:
		return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
}

DXGI_FORMAT DX12Factory::Translation(BearRenderTargetFormat format)
{
	switch (format)
	{
	case RTF_NONE:
		BEAR_CHECK(0);
		break;
	case RTF_R8:
		return DXGI_FORMAT_R8_UNORM;
		break;
	case RTF_R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
		break;

	case RTF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case RTF_B8G8R8A8:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case RTF_R32F:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case RTF_R32G32F:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case RTF_R32G32B32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case RTF_R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		BEAR_CHECK(0);
		break;
	}
	return DXGI_FORMAT_R32G32B32A32_FLOAT;
}

DXGI_FORMAT DX12Factory::Translation(BearDepthStencilFormat format)
{
	switch (format)
	{
	case DSF_DEPTH16:
		return DXGI_FORMAT_D16_UNORM;
		break;
	case DSF_DEPTH32F:
		return DXGI_FORMAT_D32_FLOAT;
		break;
	case DSF_DEPTH24_STENCIL8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	case DSF_DEPTH32F_STENCIL8:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		break;
	default:
		BEAR_CHECK(0);
		break;
	}
	return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

bool DX12Factory::SupportRayTracing()
{
#ifdef DX12_1
	return bSupportRayTracing;
#elif DX12
	return bSupportRayTracing;
#else
	return false;
#endif
}

bool DX12Factory::SupportMeshShader()
{
#ifdef DX12_1
	return bSupportMeshShader;
#elif DX12
	return bSupportMeshShader;
#else
	return false;
#endif
}
