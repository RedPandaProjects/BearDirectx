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

	virtual void Copy(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIIndexBuffer> Src);
	virtual void Copy(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Src);
	virtual void Copy(BearFactoryPointer<BearRHI::BearRHIUniformBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> Src);

	virtual void SetPipeline(BearFactoryPointer<BearRHI::BearRHIPipeline> Pipeline);
	virtual void SetVertexBuffer(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> buffer);
	virtual void SetIndexBuffer(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> buffer);
	virtual void SetViewport(float x, float y, float width, float height, float minDepth = 0.f, float maxDepth = 1.f);
	virtual void SetScissor(bool Enable, float x, float y, float x1, float y1);
	virtual void Draw(bsize count, bsize offset = 0);
	virtual void DrawIndex(bsize count, bsize  offset_index = 0, bsize  offset_vertex = 0);

	virtual void SetDescriptorHeap(BearFactoryPointer<BearRHI::BearRHIDescriptorHeap> DescriptorHeap);

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