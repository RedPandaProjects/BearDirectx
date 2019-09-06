#include "DX12PCH.h"

DX12RenderViewport::DX12RenderViewport(void * Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync)
{

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	R_CHK(Factory->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)));

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
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

		RtvDescriptorSize = Factory->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FrameCount; n++)
		{
			R_CHK(SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n])));
			Factory->Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, RtvDescriptorSize);
		}
	}
	R_CHK(Factory->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)));

	R_CHK(CommandAllocator->Reset());
	R_CHK(Factory->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList)));
	CommandList->Close();
	R_CHK(CommandList->Reset(CommandAllocator.Get(), 0));


	R_CHK(Factory->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
	FenceValue = 1;

	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (FenceEvent == nullptr)
	{
		R_CHK(HRESULT_FROM_WIN32(GetLastError()));
	}
}

DX12RenderViewport::~DX12RenderViewport()
{
	Wait();
	CloseHandle(FenceEvent);
}

void DX12RenderViewport::Flush(bool wait)
{
	R_CHK(CommandList->Close());
	Wait();
	ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	R_CHK(SwapChain->Present(1, 0));

	R_CHK(CommandQueue->Signal(Fence.Get(), ++FenceValue));
	R_CHK(CommandAllocator->Reset());
	R_CHK(CommandList->Reset(CommandAllocator.Get(), 0));
	if (wait)Wait();
}

void DX12RenderViewport::Wait()
{
	if (Fence->GetCompletedValue() < FenceValue)
	{
		R_CHK(Fence->SetEventOnCompletion(FenceValue, FenceEvent));
		WaitForSingleObject(FenceEvent, INFINITE);
	}

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
}

void DX12RenderViewport::SetVSync(bool Sync)
{
}

void DX12RenderViewport::SetFullScreen(bool FullScreen)
{
}

void DX12RenderViewport::Resize(bsize Width, bsize Height)
{
}

void * DX12RenderViewport::GetRenderTarget()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart(), FrameIndex, RtvDescriptorSize);
	RtvHandle = rtvHandle;
	return &RtvHandle;
}
