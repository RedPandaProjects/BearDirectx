#pragma once
class DX12Context :public BearRHI::BearRHIContext
{
public:
	BEAR_CLASS_WITHOUT_COPY(DX12Context);
	DX12Context();
	virtual ~DX12Context();
	virtual void Wait();
	virtual void Flush(bool wait);
	virtual void AttachViewportAsFrameBuffer(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport);
	
	virtual void DetachFrameBuffer();
	virtual void ClearFrameBuffer();
private:
	inline bool Empty() { return m_viewport.empty() ; }
	BearFactoryPointer<BearRHI::BearRHIViewport> m_viewport;

	int8 m_Status;
	void PreDestroy();
	CD3DX12_VIEWPORT m_viewportRect;
	CD3DX12_RECT m_scissorRect;
private:
	void AllocCommandList();
#ifdef RTX
	ComPtr<ID3D12GraphicsCommandList4> CommandList;
#else
	ComPtr<ID3D12GraphicsCommandList> CommandList;
#endif
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

};