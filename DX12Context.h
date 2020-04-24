#pragma once
class DX12Context :public BearRHI::BearRHIContext
{
public:
	DX12Context();
	virtual ~DX12Context();

	virtual void BeginEvent(char const* name, BearColor color = BearColor::White);
	virtual void EndEvent();

	virtual void Reset();
	virtual void Wait();
	virtual void Flush(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport,bool wait);
	virtual void Flush(bool wait);
	virtual void ClearState();

	virtual void Copy(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIIndexBuffer> Src);
	virtual void Copy(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Src);
	virtual void Copy(BearFactoryPointer<BearRHI::BearRHIUniformBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> Src);
	virtual void Copy(BearFactoryPointer<BearRHI::BearRHITexture2D> Dst, BearFactoryPointer<BearRHI::BearRHITexture2D> Src);

	virtual void SetViewportAsFrameBuffer(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport);
	virtual void SetFrameBuffer(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> FrameBuffer);
	virtual void SetPipeline(BearFactoryPointer<BearRHI::BearRHIPipeline> Pipeline);
	virtual void SetDescriptorHeap(BearFactoryPointer<BearRHI::BearRHIDescriptorHeap> DescriptorHeap);
	virtual void SetVertexBuffer(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> buffer);
	virtual void SetIndexBuffer(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> buffer);
	virtual void SetViewport(float x, float y, float width, float height, float minDepth = 0.f, float maxDepth = 1.f);
	virtual void SetScissor(bool Enable, float x, float y, float x1, float y1);
	virtual void SetStencilRef(uint32 ref);
	virtual void Draw(bsize count, bsize offset = 0);
	virtual void DrawIndex(bsize count, bsize  offset_index = 0, bsize  offset_vertex = 0);
	virtual void DispatchMesh(bsize CountX, bsize CountY, bsize CountZ);

	virtual void Lock(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport);
	virtual void Unlock(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport);
	virtual void Lock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> FrameBuffer);
	virtual void Unlock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> FrameBuffer);


private:
#ifdef DX12_1
#ifdef DX12UTIMATE
	ComPtr<ID3D12GraphicsCommandList6> m_commandList;
#else
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;
#endif
#else
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
#endif
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	uint64 m_fenceValue;
	D3D12_RECT m_ScissorRect;
};