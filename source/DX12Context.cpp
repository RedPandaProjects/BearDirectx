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
    if (Empty())
        return;
    if (m_Status != 2)
        return;

    if (m_fence->GetCompletedValue() < m_fenceValue - 1)
    {
        R_CHK(m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
    R_CHK(m_commandAllocator->Reset());
    R_CHK(CommandList->Reset(m_commandAllocator.Get(), 0));
    static_cast<DX12Viewport *>(m_viewport.get())->ToRT(CommandList.Get());
    m_Status = 0;
}

void DX12Context::Flush(bool wait)
{
    if (Empty())
        return;
    if (m_Status != 1)
        return;

    ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };

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

   /* CommandList->SetGraphicsRootSignature(Factory->RootSignature.Get());
    CommandList->RSSetViewports(1, &m_viewportRect);
    CommandList->RSSetScissorRects(1, &m_scissorRect);*/
}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Src)
{
    if (m_Status == 2)return;
    if (Dst.empty() || Src.empty())return;
    if (static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get() == nullptr)return;
    if (static_cast<DX12VertexBuffer*>(Src.get())->VertexBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    CommandList->ResourceBarrier(1, &var1);
    CommandList->CopyBufferRegion(static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get(), 0, static_cast<DX12VertexBuffer*>(Src.get())->VertexBuffer.Get(), 0, static_cast<DX12VertexBuffer*>(Dst.get())->VertexBufferView.SizeInBytes);
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    CommandList->ResourceBarrier(1, &var2);
 
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
}

void DX12Context::PreDestroy()
{
    if (Empty())
        return;
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
