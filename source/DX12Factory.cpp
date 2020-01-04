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
#ifdef RTX
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
}

DX12Factory::~DX12Factory()
{
#ifdef RTX
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
BearRHI::BearRHIViewport *DX12Factory::CreateViewport(void* Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearRenderViewportDescription &Description)
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

BearRHI::BearRHIVertexBuffer* DX12Factory::CreateVertexBuffer(bsize stride, bsize count, bool dynamic)
{
	return nullptr;
}

BearRHI::BearRHIIndexBuffer* DX12Factory::CreateIndexBuffer(bsize count, bool dynamic)
{
	return nullptr;
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
