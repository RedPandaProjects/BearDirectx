#pragma once
class DX12Viewport :public BearRHI::BearRHIViewport
{
public:
	BEAR_CLASS_WITHOUT_COPY(DX12Viewport);
	DX12Viewport(void * Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearViewportDescription&Description);
	virtual ~DX12Viewport();
	virtual void SetVSync(bool Sync);
	virtual void SetFullScreen(bool FullScreen);
	virtual void Resize(bsize Width, bsize Height);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetHandle();
	void Swap();
	virtual BearRenderTargetFormat GetFormat();
	bool m_is_rt;
	void ToPresent(ID3D12GraphicsCommandList*Cmd);
	void ToRT(ID3D12GraphicsCommandList* Cmd);

	BearViewportDescription Description;
	ComPtr<ID3D12CommandQueue> CommandQueue;
private:
	void ReInit(bsize Width, bsize Height);

	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_RtvHeap;

	constexpr static bsize FrameCount = 2;
	UINT m_FrameIndex;
	ComPtr<ID3D12Resource> m_RenderTargets[FrameCount];

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_RtvHandle;


	DXGI_SWAP_CHAIN_DESC1 m_swapChainDesc = {};
	bool m_Fullscreen;
	bool m_VSync;
	bsize m_Width;
	bsize m_Height;
};