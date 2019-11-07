#include "DX12PCH.h"
DX12RenderFactory::DX12RenderFactory()
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

	GetHardwareAdapter(GIFactory.Get(), &hardwareAdapter);
	if (hardwareAdapter.Get() == 0)return;
	IDXGIOutput *Output;
	hardwareAdapter->EnumOutputs(0, &Output);
	{
		UINT count = 0;

		Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, 0);
		m_GIVideoMode.resize(count);
		Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, &m_GIVideoMode[0]);
	}

	Output->Release();
	if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device))))
	{
		return;
	}
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
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
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
	{
		CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc;
		RootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		R_CHK(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		R_CHK(Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
	}

	{
		m_Copy_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_Copy_FenceEvent == nullptr)
		{
			R_CHK(HRESULT_FROM_WIN32(GetLastError()));
		}
		R_CHK(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_Copy_CommandAllocator)));
		R_CHK(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_Copy_CommandAllocator.Get(), 0, IID_PPV_ARGS(&CopyCommandList)));
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
			R_CHK(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Copy_CommandQueue)));
		}
		R_CHK(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Copy_Fence)));
		m_Copy_FenceValue = 1;
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
	CbvSrvUavDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	SamplerDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	{

		R_CHK(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DxcCompiler)));
		R_CHK(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&DxcLibrary)));
		R_CHK(DxcLibrary->CreateIncludeHandler(&DxcIncludeHandler));
		
	}
}
DX12RenderFactory::~DX12RenderFactory()
{
	{
		DxcIncludeHandler->Release();
	}
	{
		R_CHK(CopyCommandList->Close());
		if (m_Copy_FenceEvent)  CloseHandle(m_Copy_FenceEvent);
		m_Copy_FenceEvent = 0;
	}
	{
		R_CHK(CommandList->Close());
		if (m_Default_FenceEvent)  CloseHandle(m_Default_FenceEvent);
		m_Default_FenceEvent = 0;
	}
}
BearRenderBase::BearRenderInterfaceBase* DX12RenderFactory::CreateInterface()
{
	return bear_new<DX12RenderInterface>();
}
BearRenderBase::BearRenderContextBase * DX12RenderFactory::CreateContext()
{
	return bear_new<DX12RenderContext>();;
}
BearRenderBase::BearRenderViewportBase * DX12RenderFactory::CreateViewport(void * Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearGraphics::BearRenderViewportDescription&Description)
{
	return  bear_new<DX12RenderViewport>(Handle, Width, Height, Fullscreen, VSync, Description);;;
}

BearRenderBase::BearRenderShaderBase * DX12RenderFactory::CreateShader(BearGraphics::BearShaderType Type)
{
	return  bear_new<DX12RenderShader>(Type);
}

BearRenderBase::BearRenderPipelineBase * DX12RenderFactory::CreatePipeline(const BearGraphics::BearRenderPipelineDescription & Descruotion)
{
	return  bear_new<DX12RenderPipeline>(Descruotion);
}

BearRenderBase::BearRenderIndexBufferBase * DX12RenderFactory::CreateIndexBuffer()
{
	return bear_new<DX12RenderIndexBuffer>();
}

BearRenderBase::BearRenderVertexBufferBase * DX12RenderFactory::CreateVertexBuffer()
{
	return  bear_new<DX12RenderVertexBuffer>();
}

BearRenderBase::BearRenderUniformBufferBase * DX12RenderFactory::CreateUniformBuffer()
{
	return  bear_new<DX12RenderUniformBuffer>();
}

BearRenderBase::BearRenderRootSignatureBase * DX12RenderFactory::CreateRootSignature(const BearGraphics::BearRenderRootSignatureDescription&Description)
{
	return bear_new<DX12RenderRootSignature>(Description);
}

BearRenderBase::BearRenderDescriptorHeapBase * DX12RenderFactory::CreateDescriptorHeap(const BearGraphics::BearRenderDescriptorHeapDescription & Description)
{
	return bear_new<DX12RenderDescriptorHeap>(Description);
}

BearRenderBase::BearRenderSamplerStateBase* DX12RenderFactory::CreateSamplerState(const BearGraphics::BearRenderSamplerDescription & Description)
{
	return bear_new<DX12RenderSamplerState>(Description);
}

BearRenderBase::BearRenderTexture2DBase * DX12RenderFactory::CreateTexture2D()
{
	return bear_new<DX12RenderTexture2D>();;
}

BearRenderBase::BearRenderTexture2DUAVBase * DX12RenderFactory::CreateTexture2DUAV()
{
	return bear_new<DX12RenderTexture2DUAV>();;
}

BearRenderBase::BearRenderTargetViewBase * DX12RenderFactory::CreateTargetView(const BearGraphics::BearRenderTargetViewDescription & Description)
{
	return bear_new<DX12RenderTargetView>(Description);;
}

BearRenderBase::BearRenderFrameBufferBase * DX12RenderFactory::CreateFrameBuffer(const BearGraphics::BearRenderFrameBufferDescription & Description)
{
	return bear_new<DX12RenderFrameBuffer>(Description);;
}



DXGI_FORMAT DX12RenderFactory::Translation(BearGraphics::BearTextureUAVPixelFormat format)
{
	switch (format)
	{

	case BearGraphics::TPUF_R8:
		return DXGI_FORMAT_R8_UNORM;
		break;
	case BearGraphics::TPUF_R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
		break;
	case BearGraphics::TPUF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case BearGraphics::TPUF_R32F:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case BearGraphics::TPUF_R32G32F:
		return DXGI_FORMAT_R32G32_UINT;
		break;
	case BearGraphics::TPUF_R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		BEAR_ASSERT(0);;
		break;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT DX12RenderFactory::Translation(BearGraphics::BearTexturePixelFormat format)
{
	switch (format)
	{
	case BearGraphics::TPF_R8:
		return DXGI_FORMAT_R8_UNORM;
		break;
	case BearGraphics::TPF_R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
		break;
	case BearGraphics::TPF_R8G8B8:
		BEAR_RASSERT(!"not support R8G8B8");
		break;
	case BearGraphics::TPF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case BearGraphics::TPF_R32F:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case BearGraphics::TPF_R32G32F:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case BearGraphics::TPF_R32G32B32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case BearGraphics::TPF_R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case BearGraphics::TPF_BC1:
	case BearGraphics::TPF_BC1a:
		return DXGI_FORMAT_BC1_UNORM;
	case BearGraphics::TPF_BC2:
		return DXGI_FORMAT_BC2_UNORM;
	case BearGraphics::TPF_BC3:
		return DXGI_FORMAT_BC3_UNORM;
	case BearGraphics::TPF_BC4:
		return DXGI_FORMAT_BC4_UNORM;
	case BearGraphics::TPF_BC5:
		return DXGI_FORMAT_BC5_UNORM;
	case BearGraphics::TPF_BC6:
		return DXGI_FORMAT_BC6H_UF16;
	case BearGraphics::TPF_BC7:
		return DXGI_FORMAT_BC7_UNORM;
	default:
		BEAR_ASSERT(0);;
	}
	return DXGI_FORMAT_UNKNOWN;

}

DXGI_FORMAT DX12RenderFactory::Translation(BearGraphics::BearRenderTargetFormat format)
{
	switch (format)
	{
	case BearGraphics::RTF_NONE:
		break;
	case BearGraphics::RTF_R8:
		return DXGI_FORMAT_R8_UNORM;
		break;
	case BearGraphics::RTF_R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
		break;
	case BearGraphics::RTF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case BearGraphics::RTF_R32F:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case BearGraphics::RTF_R32G32F:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case BearGraphics::RTF_R32G32B32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case BearGraphics::RTF_R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		BEAR_ASSERT(0);;
		break;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_MODE_DESC * DX12RenderFactory::FindMode(bsize width, bsize height)
{
	auto begin = m_GIVideoMode.begin();
	auto end = m_GIVideoMode.end();
	while (begin != end)
	{
		if (begin->Width == width && begin->Height == height)
			return &*begin;
		begin++;
	}
	return 0;
}

void DX12RenderFactory::LockCopyCommandList()
{
	m_Copy_CommandMutex.Lock();
}

void DX12RenderFactory::UnlockCopyCommandList()
{
	R_CHK(CopyCommandList->Close());
	ID3D12CommandList* ppCommandLists[] = { CopyCommandList.Get() };
	m_Copy_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	const UINT64 fence = m_Copy_FenceValue;
	R_CHK(m_Copy_CommandQueue->Signal(m_Copy_Fence.Get(), fence));
	m_Copy_FenceValue++;

	if (m_Copy_Fence->GetCompletedValue() < fence)
	{
		R_CHK(m_Copy_Fence->SetEventOnCompletion(fence, m_Copy_FenceEvent));
		WaitForSingleObject(m_Copy_FenceEvent, INFINITE);
	}
	R_CHK(m_Copy_CommandAllocator->Reset());
	R_CHK(CopyCommandList->Reset(m_Copy_CommandAllocator.Get(), 0));
	m_Copy_CommandMutex.Unlock();
}

void DX12RenderFactory::LockCommandList()
{
	m_Default_CommandMutex.Lock();
}

void DX12RenderFactory::UnlockCommandList()
{
	R_CHK(CommandList->Close());
	ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
	m_Default_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	const UINT64 fence = m_Default_FenceValue;
	R_CHK(m_Default_CommandQueue->Signal(m_Default_Fence.Get(), fence));
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



void DX12RenderFactory::GetHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter)
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
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
