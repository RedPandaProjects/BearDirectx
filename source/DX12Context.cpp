#include "DX12PCH.h"

DX12Context::DX12Context()
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
    m_Status = 0;
    AllocCommandList();
}

DX12Context::~DX12Context()
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

void DX12Context::Wait()
{

    if (m_Status != 2)
        return;

    if (m_fence->GetCompletedValue() < m_fenceValue - 1)
    {
        R_CHK(m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
    R_CHK(m_commandAllocator->Reset());
    R_CHK(CommandList->Reset(m_commandAllocator.Get(), 0));
    if (!m_viewport.empty())
        static_cast<DX12Viewport*>(m_viewport.get())->ToRT(CommandList.Get());
    m_Status = 0;
}

void DX12Context::Flush(bool wait)
{
    if (m_Status != 1)
        return;

    ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
    if (Empty())
    {
        R_CHK(CommandList->Close());
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        R_CHK(m_commandQueue->Signal(m_fence.Get(), m_fenceValue++));
    }
    else
    {
        auto viewport = static_cast<DX12Viewport*>(m_viewport.get());
        viewport->ToPresent(CommandList.Get());
        R_CHK(CommandList->Close());
        viewport->CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        viewport->Swap();
        R_CHK(viewport->CommandQueue->Signal(m_fence.Get(), m_fenceValue++));

    }
    m_Status = 2;
    if (wait)
        Wait();
}

void DX12Context::AttachViewportAsFrameBuffer(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport)
{
    DetachFrameBuffer();
    m_viewport = Viewport;
}

void DX12Context::DetachFrameBuffer()
{
    PreDestroy();
    m_viewport.clear();
}

void DX12Context::ClearFrameBuffer()
{
    if (Empty())
        return;
    if (m_Status == 2)
        Wait();
    m_Status = 1;
    {

        if (static_cast<DX12Viewport*>(m_viewport.get())->Description.Clear)
        {
           auto Handle =  static_cast<DX12Viewport*>(m_viewport.get())->GetHandle();
           CommandList->ClearRenderTargetView(Handle, static_cast<DX12Viewport*>(m_viewport.get())->Description.ClearColor.R32G32B32A32, 0, 0);
           CommandList->OMSetRenderTargets(1, &Handle, FALSE, nullptr);
        }
        
    }

   /* CommandList->SetGraphicsRootSignature(Factory->RootSignature.Get());*/
    CommandList->RSSetViewports(1, &m_viewportRect);
    CommandList->RSSetScissorRects(1, &m_scissorRect);
}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Src)
{
    if (m_Status == 2)return;
    if (Dst.empty() || Src.empty())return;
    if (static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get() == nullptr)return;
    if (static_cast<DX12VertexBuffer*>(Src.get())->VertexBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    CommandList->ResourceBarrier(1, &var1);
    CommandList->CopyBufferRegion(static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get(), 0, static_cast<DX12VertexBuffer*>(Src.get())->VertexBuffer.Get(), 0, static_cast<DX12VertexBuffer*>(Dst.get())->VertexBufferView.SizeInBytes);
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    CommandList->ResourceBarrier(1, &var2);
    m_Status = 1;
 
}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIUniformBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> Src)
{
    if (m_Status == 2)return;
    if (Dst.empty() || Src.empty())return;
    if (static_cast<DX12UniformBuffer*>(Dst.get())->UniformBuffer.Get() == nullptr)return;
    if (static_cast<DX12UniformBuffer*>(Src.get())->UniformBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12UniformBuffer*>(Dst.get())->UniformBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    CommandList->ResourceBarrier(1, &var1);
    CommandList->CopyBufferRegion(static_cast<DX12UniformBuffer*>(Dst.get())->UniformBuffer.Get(), 0, static_cast<DX12UniformBuffer*>(Src.get())->UniformBuffer.Get(), 0, static_cast<DX12UniformBuffer*>(Dst.get())->UniformBufferView.SizeInBytes);
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12UniformBuffer*>(Dst.get())->UniformBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    CommandList->ResourceBarrier(1, &var2);
    m_Status = 1;

}
void DX12Context::SetPipeline(BearFactoryPointer<BearRHI::BearRHIPipeline> Pipeline)
{
    if (Empty())return;
    if (m_Status != 1 || Pipeline.empty())return;
    static_cast<DX12Pipeline*>( Pipeline.get())->Set(CommandList.Get());
}
void DX12Context::SetVertexBuffer(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> buffer)
{
    if (Empty())return;
    if (m_Status != 1 || buffer.empty())return;
    CommandList->IASetVertexBuffers(0, 1, &static_cast<DX12VertexBuffer*>(buffer.get())->VertexBufferView);

}
void DX12Context::SetIndexBuffer(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> buffer)
{
    if (Empty())return;
    if (m_Status != 1 || buffer.empty())return;
    CommandList->IASetIndexBuffer(&static_cast<DX12IndexBuffer*>(buffer.get())->IndexBufferView);

}
void DX12Context::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
    m_viewportRect.TopLeftX = x;
    m_viewportRect.TopLeftY = y;
    m_viewportRect.Width = width;
    m_viewportRect.Height = height;

    m_viewportRect.MinDepth = minDepth;
    m_viewportRect.MaxDepth = maxDepth;
    if (Empty())return;
    if (m_Status != 1)return;
    CommandList->RSSetViewports(1, &m_viewportRect);
}
void DX12Context::SetScissor(bool Enable, float x, float y, float x1, float y1)
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
void DX12Context::Draw(bsize count, bsize offset)
{
    if (Empty())return;
    if (m_Status != 1)return;
    CommandList->DrawInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset), 0);

}
void DX12Context::DrawIndex(bsize count, bsize offset_index, bsize  offset_vertex)
{
    if (Empty())return;
    if (m_Status != 1)return;
    CommandList->DrawIndexedInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset_index), static_cast<UINT>(offset_vertex), 0);

}
void DX12Context::SetDescriptorHeap(BearFactoryPointer<BearRHI::BearRHIDescriptorHeap> DescriptorHeap)
{
    if (Empty())return;
    if (m_Status != 1 || DescriptorHeap.empty())return;
    static_cast<DX12DescriptorHeap*>(DescriptorHeap.get())->Set(CommandList.Get());// DescriptorHeap
}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIIndexBuffer> Src)
{
    if (m_Status == 2)return;
    if (Dst.empty() || Src.empty())return;
    if (static_cast<DX12IndexBuffer*>(Dst.get())->IndexBuffer.Get() == nullptr)return;
    if (static_cast<DX12IndexBuffer*>(Src.get())->IndexBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12IndexBuffer*>(Dst.get())->IndexBuffer.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    CommandList->ResourceBarrier(1, &var1);
    CommandList->CopyBufferRegion(static_cast<DX12IndexBuffer*>(Dst.get())->IndexBuffer.Get(), 0, static_cast<DX12IndexBuffer*>(Src.get())->IndexBuffer.Get(), 0, static_cast<DX12IndexBuffer*>(Dst.get())->IndexBufferView.SizeInBytes);
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12IndexBuffer*>(Dst.get())->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    CommandList->ResourceBarrier(1, &var2);
    m_Status = 1;
}

void DX12Context::PreDestroy()
{
    if (m_Status == 1)
        Flush(true);
    if (m_Status == 2)
        Wait();
}

void DX12Context::AllocCommandList()
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
