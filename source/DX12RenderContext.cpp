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
	R_CHK(m_commandList->Close());

	CloseHandle(m_fenceEvent);
}

void DX12RenderContext::Wait()
{
	if (Empty())return;
	if (m_Status != 2)return;
	if (!m_viewport.empty())
	{

		static_cast<DX12RenderViewport*>(m_viewport.get())->Wait();
	}
	else
	{
		if (m_fence->GetCompletedValue() < m_fenceValue - 1)
		{
			R_CHK(m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent));
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		R_CHK(m_commandAllocator->Reset());
		R_CHK(m_commandList->Reset(m_commandAllocator.Get(), 0));
		static_cast<DX12RenderFrameBuffer*>(m_framebuffer.get())->ToRT(GetCommandList().Get());
	}
	m_Status = 0;
}

void DX12RenderContext::Flush(bool wait)
{
	if (Empty())return;
	if (m_Status != 1)return;
	if (!m_viewport.empty())
	{

		static_cast<DX12RenderViewport*>(m_viewport.get())->Flush(wait);
		if (wait)m_Status = 0;
		else m_Status = 2;
	}
	else
	{
		static_cast<DX12RenderFrameBuffer*>(m_framebuffer.get())->ToPresent(GetCommandList().Get());
		R_CHK(m_commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		R_CHK(m_commandQueue->Signal(m_fence.Get(), m_fenceValue++));
		 m_Status = 2;
		if (wait)Wait();
	}


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
		GetCommandList()->ClearRenderTargetView(*static_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(static_cast<DX12RenderViewport*>(m_viewport.get())->GetHandle()),static_cast<DX12RenderViewport*>(m_viewport.get())->Description.ClearColor.GetFloat().array,0,0);
		GetCommandList()->OMSetRenderTargets(1, static_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(static_cast<DX12RenderViewport*>(m_viewport.get())->GetHandle()), FALSE, nullptr);
	}
	else
	{

	
		static_cast<DX12RenderFrameBuffer*>(m_framebuffer.get())->Set(GetCommandList().Get());
		static_cast<DX12RenderFrameBuffer*>(m_framebuffer.get())->Clear(GetCommandList().Get());
		
	}
	GetCommandList()->SetGraphicsRootSignature(Factory->RootSignature.Get());
	GetCommandList()->RSSetViewports(1, &m_viewportRect);
	GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
}

void DX12RenderContext::SetPipeline(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderPipelineBase> Pipeline)
{
	if (Empty())return;
	if (m_Status != 1|| Pipeline.empty())return;
	Pipeline->Set(GetCommandList().Get());
	
}

void DX12RenderContext::SetVertexBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderVertexBufferBase> buffer)
{
	if (Empty())return;
	if (m_Status != 1 || buffer.empty())return;
	GetCommandList()->IASetVertexBuffers(0,1,&static_cast<DX12RenderVertexBuffer*>(buffer.get())->VertexBufferView);

}

void DX12RenderContext::SetIndexBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderIndexBufferBase> buffer)
{
	if (Empty())return;
	if (m_Status != 1 || buffer.empty())return;
	GetCommandList()->IASetIndexBuffer(&static_cast<DX12RenderIndexBuffer*>(buffer.get())->IndexBufferView);
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
	GetCommandList()->RSSetViewports(1, &m_viewportRect);

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
		GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
	}
	else
	{
		m_scissorRect.left = 0;
		m_scissorRect.right = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		m_scissorRect.top = 0;
		m_scissorRect.bottom = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		if (m_Status != 1)return;
		if (Empty())return;
		GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
	}
}

void DX12RenderContext::Draw(bsize count, bsize offset)
{
	if (Empty())return;
	if (m_Status != 1)return;
	GetCommandList()->DrawInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset), 0);
}

void DX12RenderContext::DrawIndex(bsize count, bsize offset)
{
	if (Empty())return;
	if (m_Status != 1)return;
	GetCommandList()->DrawIndexedInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset), 0,0);
}

void DX12RenderContext::SetDescriptorHeap(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderDescriptorHeapBase> DescriptorHeap)
{
	if (Empty())return;
	if (m_Status != 1|| DescriptorHeap.empty())return;
	static_cast<DX12RenderDescriptorHeap*>(DescriptorHeap.get())->Set(GetCommandList().Get());
}

void DX12RenderContext::DispatchRays(const BearGraphics::BearRenderDispatchRaysDescription & Description)
{
}

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
	R_CHK(Factory->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	m_commandList->Close();
	R_CHK(m_commandList->Reset(m_commandAllocator.Get(), 0));


	R_CHK(Factory->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 0;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		R_CHK(HRESULT_FROM_WIN32(GetLastError()));
	}
}

