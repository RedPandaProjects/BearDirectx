#include "DX12PCH.h"

DX12Factory::DX12Factory()
{
	UINT dxgiFactoryFlags = 0;

#if defined(DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

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
	{
#if defined(_DEBUG)
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
#endif
	}

	CbvSrvUavDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	SamplerDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	{
#ifdef RTX_SHADER_COMPILER
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
#ifdef RTX_SHADER_COMPILER
	{
		DxcIncludeHandler->Release();
	}
#endif
	{
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

BearRHI::BearRHIUniformBuffer* DX12Factory::CreateUniformBuffer()
{
	return bear_new<DX12UniformBuffer>();
}

BearRHI::BearRHIRootSignature* DX12Factory::CreateRootSignature(const BearRootSignatureDescription& Description)
{
	return bear_new< DX12RootSignature>(Description);
}

BearRHI::BearRHIDescriptorHeap* DX12Factory::CreateDescriptorHeap(const BearDescriptorHeapDescription& Description)
{
	return  bear_new<DX12DescriptorHeap>(Description);;
}

BearRHI::BearRHIPipeline* DX12Factory::CreatePipeline(const BearPipelineDescription& Description)
{
	return bear_new<DX12Pipeline>(Description);
}

BearRHI::BearRHITexture2D* DX12Factory::CreateTexture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, void* data)
{
	return  bear_new<DX12Texture2D>(Width,Height,Mips,Count,PixelFormat,data);;
}

BearRHI::BearRHISampler* DX12Factory::CreateSampler()
{
	return  bear_new<DX12SamplerState>();
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
		BEAR_RASSERT(!"not support R8G8B8");
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
		BEAR_ASSERT(0);;
	}
	return DXGI_FORMAT_UNKNOWN;

}


void DX12Factory::GetHardwareAdapter(IDXGIFactory2 *pFactory, IDXGIAdapter1 **ppAdapter, D3D_FEATURE_LEVEL &Level)
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
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), Level, _uuidof(ID3D12Device), nullptr)))
		{
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
	const UINT64 fence = m_Default_FenceValue;
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
