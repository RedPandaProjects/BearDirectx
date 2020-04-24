#include "DX12PCH.h"
bsize ViewportCounter = 0;
DX12Viewport::DX12Viewport(void * Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearViewportDescription&Description_):Description(Description_), m_Fullscreen(Fullscreen),m_VSync(VSync), Width(Width),Height(Height)
{
	ViewportCounter++;
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		R_CHK(Factory->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)));

	}
	{
		m_swapChainDesc.BufferCount = FrameCount;
		m_swapChainDesc.Width = static_cast<UINT>(Width);
		m_swapChainDesc.Height = static_cast<UINT>(Height);
		m_swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		m_swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		m_swapChainDesc.SampleDesc.Count = 1;


		ComPtr<IDXGISwapChain1> swapChain;
		R_CHK(Factory->GIFactory->CreateSwapChainForHwnd(
			CommandQueue.Get(),
			reinterpret_cast<HWND>(Handle),
			&m_swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		R_CHK(swapChain.As(&m_SwapChain));
		m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
	}
	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		R_CHK(Factory->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

		
	}

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FrameCount; n++)
		{
			R_CHK(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
			Factory->Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1,Factory-> RtvDescriptorSize);
		}
	}
}

DX12Viewport::~DX12Viewport()
{	
	ViewportCounter--;

}

void DX12Viewport::ReInit(bsize Width, bsize Height)
{
	

	for (UINT n = 0; n < FrameCount; n++)
	{
		m_RenderTargets[n].Reset();
	}
	m_swapChainDesc.Width = static_cast<UINT>(Width);
	m_swapChainDesc.Height = static_cast<UINT>(Height);
	R_CHK(m_SwapChain->ResizeBuffers(
		m_swapChainDesc.BufferCount,
		m_swapChainDesc.Width,
		m_swapChainDesc.Height,
		m_swapChainDesc.Format,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		R_CHK(Factory->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));


	}

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FrameCount; n++)
		{
			R_CHK(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
			Factory->Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, Factory->RtvDescriptorSize);
		}
	}



	Width = Width;
	Height = Height;
}

void DX12Viewport::SetVSync(bool Sync)
{
	m_VSync = Sync;
}

void DX12Viewport::SetFullScreen(bool FullScreen)
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
		dmScreenSettings.dmPelsWidth = static_cast<DWORD>(Width);
		dmScreenSettings.dmPelsHeight = static_cast<DWORD>(Height);
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
	ReInit(Width, Height);
}

void DX12Viewport::Resize(bsize m_Width, bsize m_Height)
{
	if (m_Height == Height && m_Width == Width)return;
	ReInit(Width, Height);

}

CD3DX12_CPU_DESCRIPTOR_HANDLE DX12Viewport::GetHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, Factory->RtvDescriptorSize);
	return rtvHandle;
}

void DX12Viewport::Unlock(ID3D12GraphicsCommandList * CommandList)
{
	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1, &transition);
}

void DX12Viewport::Swap()
{
	R_CHK(m_SwapChain->Present(m_VSync, 0));
	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

BearRenderTargetFormat DX12Viewport::GetFormat()
{
	return RTF_R8G8B8A8;
}

void DX12Viewport::Lock(ID3D12GraphicsCommandList * CommandList)
{
	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &transition);
}
