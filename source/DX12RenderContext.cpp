#include "DX12PCH.h"

DX12RenderContext::DX12RenderContext():m_Status(0)
{
	m_scissorRect.left = 0;
	m_scissorRect.right = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	m_scissorRect.top = 0;
	m_scissorRect.bottom = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	m_viewportRect.Width = 1;
	m_viewportRect.Height = 1;
	m_viewportRect.MaxDepth = 1;
	m_viewportRect.MinDepth = 0;
	m_viewportRect.TopLeftX = 0;
	m_viewportRect.TopLeftY = 0;
	AllocCommandList();
}

DX12RenderContext::~DX12RenderContext()
{
	PreDestroy();
	if (m_fenceValue)
		if (m_fence->GetCompletedValue() < m_fenceValue - 1)
		{
			R_CHK(m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent));
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	R_CHK(CommandList->Close());

	CloseHandle(m_fenceEvent);
}

void DX12RenderContext::Wait()
{
	if (Empty())return;
	if (m_Status != 2)return;

	if (m_fence->GetCompletedValue() < m_fenceValue - 1)
	{
		R_CHK(m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	R_CHK(m_commandAllocator->Reset());
	R_CHK(CommandList->Reset(m_commandAllocator.Get(), 0));
	if (m_viewport.empty())
		static_cast<DX12RenderFrameBuffer*>(m_framebuffer.get())->ToRT(CommandList.Get());
	else
		static_cast<DX12RenderViewport*>(m_viewport.get())->ToRT(CommandList.Get());
	m_Status = 0;
}

void DX12RenderContext::Flush(bool wait)
{
	if (Empty())return;
	if (m_Status != 1)return;



	ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
	if (m_viewport.empty())
	{
		static_cast<DX12RenderFrameBuffer*>(m_framebuffer.get())->ToPresent(CommandList.Get());
		R_CHK(CommandList->Close());
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		R_CHK(m_commandQueue->Signal(m_fence.Get(), m_fenceValue++));
	}
	else
	{

		auto viewport = static_cast<DX12RenderViewport*>(m_viewport.get());
		viewport->ToPresent(CommandList.Get());
		R_CHK(CommandList->Close());
		viewport->CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		R_CHK(viewport->CommandQueue->Signal(m_fence.Get(), m_fenceValue++));
		viewport->Swap();
	}
	m_Status = 2;
	if (wait)Wait();
}


void DX12RenderContext::AttachViewportAsFrameBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderViewportBase> Viewport)
{
	DetachFrameBuffer();
	m_viewport = Viewport;
}

void DX12RenderContext::AttachFrameBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderFrameBufferBase> Framebuffer)
{
	DetachFrameBuffer();
	m_framebuffer = Framebuffer;
}

void DX12RenderContext::DetachFrameBuffer()
{
	PreDestroy();
	m_viewport.clear();
	m_framebuffer.clear();
}

void DX12RenderContext::ClearFrameBuffer()
{
	if (Empty())return;
	if (m_Status == 2)Wait();
	m_Status = 1;
	if (!m_viewport.empty())
	{
		
		if(static_cast<DX12RenderViewport*>(m_viewport.get())->Description.Clear)
		CommandList->ClearRenderTargetView(*static_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(static_cast<DX12RenderViewport*>(m_viewport.get())->GetHandle()),static_cast<DX12RenderViewport*>(m_viewport.get())->Description.ClearColor.GetFloat().array,0,0);
		CommandList->OMSetRenderTargets(1, static_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(static_cast<DX12RenderViewport*>(m_viewport.get())->GetHandle()), FALSE, nullptr);
	}
	else
	{

	
		static_cast<DX12RenderFrameBuffer*>(m_framebuffer.get())->Set(CommandList.Get());
		static_cast<DX12RenderFrameBuffer*>(m_framebuffer.get())->Clear(CommandList.Get());
		
	}
	CommandList->SetGraphicsRootSignature(Factory->RootSignature.Get());
	CommandList->RSSetViewports(1, &m_viewportRect);
	CommandList->RSSetScissorRects(1, &m_scissorRect);
}

void DX12RenderContext::SetPipeline(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderPipelineBase> Pipeline)
{
	if (Empty())return;
	if (m_Status != 1|| Pipeline.empty())return;
	Pipeline->Set(CommandList.Get());
	
}

void DX12RenderContext::SetVertexBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderVertexBufferBase> buffer)
{
	if (Empty())return;
	if (m_Status != 1 || buffer.empty())return;
	CommandList->IASetVertexBuffers(0,1,&static_cast<DX12RenderVertexBuffer*>(buffer.get())->VertexBufferView);

}

void DX12RenderContext::SetIndexBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderIndexBufferBase> buffer)
{
	if (Empty())return;
	if (m_Status != 1 || buffer.empty())return;
	CommandList->IASetIndexBuffer(&static_cast<DX12RenderIndexBuffer*>(buffer.get())->IndexBufferView);
}

void DX12RenderContext::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	
	m_viewportRect.TopLeftX = x;
	m_viewportRect.TopLeftY = y;
	m_viewportRect.Width = width;
	m_viewportRect.Height = height;

	m_viewportRect.MinDepth = minDepth;
	m_viewportRect.MaxDepth = maxDepth;
	if (Empty())return;
	if (m_Status != 1 )return;
	CommandList->RSSetViewports(1, &m_viewportRect);

}

void DX12RenderContext::SetScissor(bool Enable, float x, float y, float x1, float y1)
{

	if (Enable)
	{
		m_scissorRect.left = static_cast<LONG>(x);
		m_scissorRect.right = static_cast<LONG>(x1);
		m_scissorRect.top = static_cast<LONG>(y);
		m_scissorRect.bottom = static_cast<LONG>(y1);
		if (m_Status != 1)return;
		if (Empty())return;
		CommandList->RSSetScissorRects(1, &m_scissorRect);
	}
	else
	{
		m_scissorRect.left = 0;
		m_scissorRect.right = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		m_scissorRect.top = 0;
		m_scissorRect.bottom = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		if (m_Status != 1)return;
		if (Empty())return;
		CommandList->RSSetScissorRects(1, &m_scissorRect);
	}
}

void DX12RenderContext::Draw(bsize count, bsize offset)
{
	if (Empty())return;
	if (m_Status != 1)return;
	CommandList->DrawInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset), 0);
}

void DX12RenderContext::DrawIndex(bsize count, bsize offset)
{
	if (Empty())return;
	if (m_Status != 1)return;
	CommandList->DrawIndexedInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset), 0,0);
}

void DX12RenderContext::SetDescriptorHeap(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderDescriptorHeapBase> DescriptorHeap)
{
	if (Empty())return;
	if (m_Status != 1|| DescriptorHeap.empty())return;
	static_cast<DX12RenderDescriptorHeap*>(DescriptorHeap.get())->Set(CommandList.Get());
}
/*
void DX12RenderContext::DispatchRays(const BearGraphics::BearRenderDispatchRaysDescription & Description)
{
	D3D12_DISPATCH_RAYS_DESC Desc = {};
	Desc.Width = static_cast<UINT>(Description.Width);
	Desc.Height = static_cast<UINT>(Description.Height);
	Desc.Depth = static_cast<UINT>(Description.Depth);

	if (!Description.RayGeneration.Buffer.empty())
	{
		auto buffer = static_cast<const DX12RenderUniformBuffer*>(Description.RayGeneration.Buffer.get());
		Desc.RayGenerationShaderRecord.StartAddress = buffer->UniformBuffer->GetGPUVirtualAddress();
		Desc.RayGenerationShaderRecord.SizeInBytes = static_cast<UINT>(Description.RayGeneration.Size);
	}
	if (!Description.Miss.Buffer.empty())
	{
		auto buffer = static_cast<const DX12RenderUniformBuffer*>(Description.Miss.Buffer.get());
		Desc.MissShaderTable.StartAddress = buffer->UniformBuffer->GetGPUVirtualAddress();
		Desc.MissShaderTable.SizeInBytes = static_cast<UINT>(Description.Miss.Size);
		Desc.MissShaderTable.StrideInBytes = static_cast<UINT>(Description.Miss.Stride);
	}
	if (!Description.HitGroup.Buffer.empty())
	{
		auto buffer = static_cast<const DX12RenderUniformBuffer*>(Description.HitGroup.Buffer.get());
		Desc.HitGroupTable.StartAddress = buffer->UniformBuffer->GetGPUVirtualAddress();
		Desc.HitGroupTable.SizeInBytes = static_cast<UINT>(Description.HitGroup.Size);
		Desc.HitGroupTable.StrideInBytes = static_cast<UINT>(Description.HitGroup.Stride);
	}
	if (!Description.Callable.Buffer.empty())
	{
		auto buffer = static_cast<const DX12RenderUniformBuffer*>(Description.Callable.Buffer.get());
		Desc.CallableShaderTable.StartAddress = buffer->UniformBuffer->GetGPUVirtualAddress();
		Desc.CallableShaderTable.SizeInBytes = static_cast<UINT>(Description.Callable.Size);
		Desc.CallableShaderTable.StrideInBytes = static_cast<UINT>(Description.Callable.Stride);
	}
	CommandList->DispatchRays(&Desc);
}
*/
void DX12RenderContext::PreDestroy()
{
	if (Empty())return;
	if (m_Status==1)Flush(true);
	if (m_Status == 2)Wait();

}

void DX12RenderContext::AllocCommandList()
{

	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		R_CHK(Factory->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	}
	R_CHK(Factory->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	R_CHK(m_commandAllocator->Reset());
	R_CHK(Factory->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList)));
	CommandList->Close();
	R_CHK(CommandList->Reset(m_commandAllocator.Get(), 0));


	R_CHK(Factory->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 0;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		R_CHK(HRESULT_FROM_WIN32(GetLastError()));
	}
}

