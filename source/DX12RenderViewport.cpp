#include "DX12PCH.h"

DX12RenderViewport::DX12RenderViewport(void * Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearGraphics::BearRenderViewportDescription&Description_):Description(Description_), m_Fullscreen(Fullscreen),m_VSync(VSync), m_Width(Width),m_Height(Height)
{
	m_wait = false;
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	R_CHK(Factory->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)));

	// Describe and create the swap chain.

	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = static_cast<UINT>(Width);
	swapChainDesc.Height = static_cast<UINT>(Height);
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;


	ComPtr<IDXGISwapChain1> swapChain;
	R_CHK(Factory->GIFactory->CreateSwapChainForHwnd(
		CommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		reinterpret_cast<HWND>(Handle),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	R_CHK(swapChain.As(&SwapChain));
	FrameIndex = SwapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		R_CHK(Factory->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RtvHeap)));

		
	}

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FrameCount; n++)
		{
			R_CHK(SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n])));
			Factory->Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1,Factory-> RtvDescriptorSize);
		}
	}
	R_CHK(Factory->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)));

	R_CHK(CommandAllocator->Reset());
	R_CHK(Factory->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList)));
	CommandList->Close();
	R_CHK(CommandList->Reset(CommandAllocator.Get(), 0));


	R_CHK(Factory->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
	FenceValue = 0;

	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (FenceEvent == nullptr)
	{
		R_CHK(HRESULT_FROM_WIN32(GetLastError()));
	}
	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &transition); m_Status = 1;

}

DX12RenderViewport::~DX12RenderViewport()
{	
	if(FenceValue)
	if (Fence->GetCompletedValue() < FenceValue - 1)
	{
		R_CHK(Fence->SetEventOnCompletion(FenceValue - 1, FenceEvent));
		WaitForSingleObject(FenceEvent, INFINITE);
	}
	R_CHK(CommandList->Close());

	CloseHandle(FenceEvent);
}

void DX12RenderViewport::Flush(bool wait)
{
	m_Status = 1;
	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1,&transition);

	R_CHK(CommandList->Close());

	ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	R_CHK(SwapChain->Present(m_VSync, 0));

	R_CHK(CommandQueue->Signal(Fence.Get(), FenceValue++));
	if (wait)Wait();
}

void DX12RenderViewport::Wait()
{
	if (m_Status != 1)return;
	if (FenceValue == 0)return;
	m_Status = 0;
	if (Fence->GetCompletedValue() < FenceValue-1)
	{
		R_CHK(Fence->SetEventOnCompletion(FenceValue-1, FenceEvent));
		WaitForSingleObject(FenceEvent, INFINITE);
	}
	R_CHK(CommandAllocator->Reset());
	R_CHK(CommandList->Reset(CommandAllocator.Get(), 0));
	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &transition);

	m_wait = false;
}

void DX12RenderViewport::ReInit(bsize Width, bsize Height)
{
	if (m_Status == 1)
	{
		Wait();
	}
	R_CHK(CommandList->Close());
	R_CHK(CommandAllocator->Reset());
	R_CHK(CommandList->Reset(CommandAllocator.Get(), 0));
	m_Status = 0;
	

	for (UINT n = 0; n < FrameCount; n++)
	{
		RenderTargets[n].Reset();
	}
	swapChainDesc.Width = static_cast<UINT>(Width);
	swapChainDesc.Height = static_cast<UINT>(Height);
	R_CHK(SwapChain->ResizeBuffers(
		swapChainDesc.BufferCount,
		swapChainDesc.Width,
		swapChainDesc.Height,
		swapChainDesc.Format,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		R_CHK(Factory->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RtvHeap)));


	}

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FrameCount; n++)
		{
			R_CHK(SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n])));
			Factory->Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, Factory->RtvDescriptorSize);
		}
	}


	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &transition);
	m_Width = Width;
	m_Height = Height;
}

void DX12RenderViewport::SetVSync(bool Sync)
{
	m_VSync = Sync;
}

void DX12RenderViewport::SetFullScreen(bool FullScreen)
{
	if (m_Fullscreen == FullScreen)return;
	if (FullScreen)
	{
		/*HMONITOR hMonitor = MonitorFromWindow((HWND)m_WindowHandle, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFOEX MonitorInfo;
		memset(&MonitorInfo, 0, sizeof(MONITORINFOEX));
		MonitorInfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(hMonitor, &MonitorInfo);

		DEVMODE Mode;
		Mode.dmSize = sizeof(DEVMODE);
		Mode.dmBitsPerPel = 32;
		Mode.dmPelsWidth = static_cast<DWORD>(Width);
		Mode.dmPelsHeight = static_cast<DWORD>(Height);
		Mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettingsEx(MonitorInfo.szDevice, &Mode, NULL, CDS_FULLSCREEN, NULL);*/
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = static_cast<DWORD>(m_Width);
		dmScreenSettings.dmPelsHeight = static_cast<DWORD>(m_Height);
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);


	}
	else
	{
		ChangeDisplaySettings(NULL, 0);
	}
	m_Fullscreen = FullScreen;
	ReInit(m_Width, m_Height);
}

void DX12RenderViewport::Resize(bsize Width, bsize Height)
{
	if (m_Height == Height && m_Width == Width)return;
	ReInit(Width, Height);

}

void * DX12RenderViewport::GetHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart(), FrameIndex, Factory->RtvDescriptorSize);
	RtvHandle = rtvHandle;
	return &RtvHandle;
}
