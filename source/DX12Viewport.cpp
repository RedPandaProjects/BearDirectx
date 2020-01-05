#include "DX12PCH.h"

DX12Viewport::DX12Viewport(void * Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearViewportDescription&Description_):Description(Description_), m_Fullscreen(Fullscreen),m_VSync(VSync), m_Width(Width),m_Height(Height)
{
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
	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	Factory->LockCommandList();
	Factory->CommandList->ResourceBarrier(1, &transition);
	Factory->UnlockCommandList(CommandQueue.Get());
	m_is_rt = true;

}

DX12Viewport::~DX12Viewport()
{	

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


	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	Factory->LockCommandList();
	Factory->CommandList->ResourceBarrier(1, &transition);
	Factory->UnlockCommandList();
	m_Width = Width;
	m_Height = Height;
	m_is_rt = true;
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

void DX12Viewport::Resize(bsize Width, bsize Height)
{
	if (m_Height == Height && m_Width == Width)return;
	ReInit(Width, Height);

}

CD3DX12_CPU_DESCRIPTOR_HANDLE DX12Viewport::GetHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, Factory->RtvDescriptorSize);
	return rtvHandle;
}


void DX12Viewport::ToPresent(ID3D12GraphicsCommandList * CommandList)
{
	if (!m_is_rt)return;
	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1, &transition);
	m_is_rt = false;
}

void DX12Viewport::Swap()
{
	R_CHK(m_SwapChain->Present(m_VSync, 0));
}

void DX12Viewport::ToRT(ID3D12GraphicsCommandList * CommandList)
{
	if (m_is_rt)return;
	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &transition);
	m_is_rt = true;
}
