#include "DX12PCH.h"
#define USE_PIX
#include "pix3.h"
#pragma optimize( "", off )
bsize ContextCounter = 0;
extern bool GDebugRender;
DX12Context::DX12Context()
{
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        R_CHK(Factory->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    }
    R_CHK(Factory->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
    R_CHK(Factory->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    m_commandList->Close();
    
    R_CHK(Factory->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    m_fenceValue = 1;
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        R_CHK(HRESULT_FROM_WIN32(GetLastError()));
    }
    m_CurrentPipelineIsCompute = false;
    ContextCounter++;
}
DX12Context::~DX12Context()
{
    ContextCounter--;
    if (m_fenceValue)
        if (m_fence->GetCompletedValue() < m_fenceValue - 1)
        {
            R_CHK(m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent));
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    //R_CHK(m_commandList->Close());
    CloseHandle(m_fenceEvent);
}

void DX12Context::BeginEvent(char const* name, BearColor color)
{
    if (GDebugRender)
    {
        PIXBeginEvent(m_commandList.Get(), PIX_COLOR(color.R8U, color.G8U, color.B8U), name);
    }
}
void DX12Context::EndEvent()
{
    if (GDebugRender)
    {
        PIXEndEvent(m_commandList.Get());
    }

}

void DX12Context::Reset()
{
    m_ScissorRect.left = 0;
    m_ScissorRect.right = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    m_ScissorRect.top = 0;
    m_ScissorRect.bottom = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator.Get(), nullptr);
    
}
void DX12Context::Wait()
{
    if (m_fence->GetCompletedValue() < m_fenceValue - 1)
    {
        R_CHK(m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}
void DX12Context::Flush(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport, bool wait)
{
    m_commandList->Close();
    ID3D12CommandList* ppm_commandLists[] = { m_commandList.Get() };
    auto Vw = static_cast<DX12Viewport*>(Viewport.get());
    Vw->CommandQueue->ExecuteCommandLists(_countof(ppm_commandLists), ppm_commandLists);
    Vw->Swap();
    Vw->CommandQueue->Signal(m_fence.Get(), m_fenceValue++);
    if (wait)
        Wait();
}
void DX12Context::Flush(bool wait)
{
    m_commandList->Close();
    ID3D12CommandList* ppm_commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppm_commandLists), ppm_commandLists);
    R_CHK(m_commandQueue->Signal(m_fence.Get(), m_fenceValue++));
    if (wait)
        Wait();
}

void DX12Context::ClearState()
{
    //m_commandList->ClearState();
}

void DX12Context::SetPipeline(BearFactoryPointer<BearRHI::BearRHIPipeline> Pipeline)
{
    DX12Pipeline* P = reinterpret_cast<DX12Pipeline*>(Pipeline.get()->QueryInterface(DX12Q_Pipeline));
    BEAR_CHECK(P);
    P->Set(m_commandList.Get());
    m_CurrentPipelineIsCompute = P->IsComputePipeline();
}
void DX12Context::SetViewportAsFrameBuffer(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport)
{

    BEAR_CHECK(!Viewport.empty());

    auto Handle = static_cast<DX12Viewport*>(Viewport.get())->GetHandle();
    if (static_cast<DX12Viewport*>(Viewport.get())->Description.Clear)
    {
        D3D12_RECT Rect = { 0,0,static_cast<LONG>(static_cast<DX12Viewport*>(Viewport.get())->Width),static_cast<LONG>(static_cast<DX12Viewport*>(Viewport.get())->Height) };
        m_commandList->ClearRenderTargetView(Handle, static_cast<DX12Viewport*>(Viewport.get())->Description.ClearColor.R32G32B32A32, 1, &Rect);
    }
    m_commandList->OMSetRenderTargets(1, &Handle, FALSE, nullptr);

}
void DX12Context::SetFrameBuffer(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> FrameBuffer)
{
    BEAR_CHECK(!FrameBuffer.empty());
    auto framebuffer = static_cast<DX12FrameBuffer*>(FrameBuffer.get());

    D3D12_RECT Rect = { 0,0,static_cast<LONG>(framebuffer->Width),static_cast<LONG>(framebuffer->Height) };
    for (bsize i = 0; i < framebuffer->Count; i++)
    {
        if (framebuffer->RenderPassRef->Description.RenderTargets[i].Clear)
            m_commandList->ClearRenderTargetView(framebuffer->RenderTargetRefs[i], framebuffer->RenderPassRef->Description.RenderTargets[i].Color.R32G32B32A32, 1, &Rect);
    }
    if (!framebuffer->Description.DepthStencil.empty() && framebuffer->RenderPassRef->Description.DepthStencil.Clear)
    {
        D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH;

        switch (framebuffer->RenderPassRef->Description.DepthStencil.Format)
        {
        case DSF_DEPTH24_STENCIL8:
        case DSF_DEPTH32F_STENCIL8:
            flags |= D3D12_CLEAR_FLAG_STENCIL;
        default:
            break;
        }
        m_commandList->ClearDepthStencilView(framebuffer->DepthStencilRef, flags, framebuffer->RenderPassRef->Description.DepthStencil.Depth, framebuffer->RenderPassRef->Description.DepthStencil.Stencil, 1, &Rect);
    }
    m_commandList->OMSetRenderTargets(static_cast<UINT>(framebuffer->Count), framebuffer->RenderTargetRefs, FALSE, framebuffer->Description.DepthStencil.empty() ? nullptr : &framebuffer->DepthStencilRef);

}
void DX12Context::SetDescriptorHeap(BearFactoryPointer<BearRHI::BearRHIDescriptorHeap> DescriptorHeap)
{
    if (m_CurrentPipelineIsCompute)
    {
        static_cast<DX12DescriptorHeap*>(DescriptorHeap.get())->SetCompute(m_commandList.Get());
    }
    else
    {
        static_cast<DX12DescriptorHeap*>(DescriptorHeap.get())->SetGraphics(m_commandList.Get());// DescriptorHeap
    }
}
void DX12Context::SetVertexBuffer(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> buffer)
{
    m_commandList->IASetVertexBuffers(0, 1, &static_cast<DX12VertexBuffer*>(buffer.get())->VertexBufferView);

}
void DX12Context::SetIndexBuffer(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> buffer)
{
    m_commandList->IASetIndexBuffer(&static_cast<DX12IndexBuffer*>(buffer.get())->IndexBufferView);

}
void DX12Context::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
    D3D12_VIEWPORT rect;
    rect.TopLeftX = x;
    rect.TopLeftY = y;
    rect.Width = width;
    rect.Height = height;

    rect.MinDepth = minDepth;
    rect.MaxDepth = maxDepth;
    m_commandList->RSSetViewports(1, &rect);
}
void DX12Context::SetScissor(bool Enable, float x, float y, float x1, float y1)
{
    if (Enable)
    {
        m_ScissorRect.left = static_cast<LONG>(x);
        m_ScissorRect.right = static_cast<LONG>(x1);
        m_ScissorRect.top = static_cast<LONG>(y);
        m_ScissorRect.bottom = static_cast<LONG>(y1);
    }
    else
    {
        m_ScissorRect.left = 0;
        m_ScissorRect.right = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        m_ScissorRect.top = 0;
        m_ScissorRect.bottom = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    }

    m_commandList->RSSetScissorRects(1, &m_ScissorRect);
}
void DX12Context::SetStencilRef(uint32 ref)
{
    m_commandList->OMSetStencilRef(ref);
}
void DX12Context::Draw(bsize count, bsize offset)
{
    if (m_ScissorRect.left - m_ScissorRect.right == 0)return;
    if (m_ScissorRect.top - m_ScissorRect.bottom == 0)return;
    m_commandList->DrawInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset), 0);
}
void DX12Context::DrawIndex(bsize count, bsize offset_index, bsize  offset_vertex)
{
    if (m_ScissorRect.left - m_ScissorRect.right == 0)return;
    if (m_ScissorRect.top - m_ScissorRect.bottom == 0)return;
    m_commandList->DrawIndexedInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset_index), static_cast<UINT>(offset_vertex), 0);

}

void DX12Context::DispatchMesh(bsize CountX, bsize CountY, bsize CountZ)
{
#ifdef DX12UTIMATE
    m_commandList->DispatchMesh(static_cast<UINT>(CountX), static_cast<UINT>(CountY), static_cast<UINT>(CountZ));
#endif
}

void DX12Context::Lock(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport)
{
    BEAR_CHECK(!Viewport.empty());
    static_cast<DX12Viewport*>(Viewport.get())->Lock(m_commandList.Get());
}
void DX12Context::Unlock(BearFactoryPointer<BearRHI::BearRHIViewport> Viewport)
{
    BEAR_CHECK(!Viewport.empty());
    static_cast<DX12Viewport*>(Viewport.get())->Unlock(m_commandList.Get());
}
void DX12Context::Lock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> FrameBuffer)
{
    BEAR_CHECK(!FrameBuffer.empty());
    static_cast<DX12FrameBuffer*>(FrameBuffer.get())->Lock(m_commandList.Get());
}
void DX12Context::Unlock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> FrameBuffer)
{
    BEAR_CHECK(!FrameBuffer.empty());
    static_cast<DX12FrameBuffer*>(FrameBuffer.get())->Unlock(m_commandList.Get());
}

void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIIndexBuffer> Src)
{
    if (Dst.empty() || Src.empty())return;
    if (static_cast<DX12IndexBuffer*>(Dst.get())->IndexBuffer.Get() == nullptr)return;
    if (static_cast<DX12IndexBuffer*>(Src.get())->IndexBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12IndexBuffer*>(Dst.get())->IndexBuffer.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    m_commandList->ResourceBarrier(1, &var1);
    m_commandList->CopyBufferRegion(static_cast<DX12IndexBuffer*>(Dst.get())->IndexBuffer.Get(), 0, static_cast<DX12IndexBuffer*>(Src.get())->IndexBuffer.Get(), 0, static_cast<DX12IndexBuffer*>(Dst.get())->IndexBufferView.SizeInBytes);
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12IndexBuffer*>(Dst.get())->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    m_commandList->ResourceBarrier(1, &var2);
}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHITexture2D> Dst, BearFactoryPointer<BearRHI::BearRHITexture2D> Src)
{
    {
        if (Dst.empty() || Src.empty())return;
        if (static_cast<DX12Texture2D*>(Dst.get())->TextureBuffer.Get() == nullptr)return;
        if (static_cast<DX12Texture2D*>(Src.get())->TextureBuffer.Get() == nullptr)return;
        auto dst = static_cast<DX12Texture2D*>(Dst.get());
        auto src = static_cast<DX12Texture2D*>(Src.get());
        if (src->TextureDesc.Width!= dst->TextureDesc.Width)return;
        if (src->TextureDesc.Height != dst->TextureDesc.Height)return;;
        if (src->TextureDesc.DepthOrArraySize != dst->TextureDesc.DepthOrArraySize)return;;
        if (dst->TextureDesc.MipLevels!= 1)
            if (src->TextureDesc.MipLevels != dst->TextureDesc.MipLevels)return;
        auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(dst->TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
        auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(src->TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
        auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(dst->TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        auto var4 = CD3DX12_RESOURCE_BARRIER::Transition(src->TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &var1);
        m_commandList->ResourceBarrier(1, &var2);
        CD3DX12_TEXTURE_COPY_LOCATION DSTBuffer[512];
        CD3DX12_TEXTURE_COPY_LOCATION SRCBuffer[512];
        for (bsize x = 0; x < dst->TextureDesc.DepthOrArraySize; x++)
            for (bsize y = 0; y < dst->TextureDesc.MipLevels; y++)
            {
                CD3DX12_TEXTURE_COPY_LOCATION DST(dst->TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(y), static_cast<UINT>(x), 0, dst->TextureDesc.MipLevels, dst->TextureDesc.DepthOrArraySize));
                CD3DX12_TEXTURE_COPY_LOCATION SRC(src->TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(y), static_cast<UINT>(x), 0, src->TextureDesc.MipLevels, src->TextureDesc.DepthOrArraySize));
                DSTBuffer[x * dst->TextureDesc.MipLevels + y] = DST;
                SRCBuffer[x * dst->TextureDesc.MipLevels + y] = SRC;
                m_commandList->CopyTextureRegion(&DSTBuffer[x * dst->TextureDesc.MipLevels + y], 0, 0, 0, &SRCBuffer[x * dst->TextureDesc.MipLevels + y], 0);
            }
        m_commandList->ResourceBarrier(1, &var3);
        m_commandList->ResourceBarrier(1, &var4);
    }
}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIVertexBuffer> Src)
{
    if (Dst.empty() || Src.empty())return;
    if (static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get() == nullptr)return;
    if (static_cast<DX12VertexBuffer*>(Src.get())->VertexBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    m_commandList->ResourceBarrier(1, &var1);
    m_commandList->CopyBufferRegion(static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get(), 0, static_cast<DX12VertexBuffer*>(Src.get())->VertexBuffer.Get(), 0, static_cast<DX12VertexBuffer*>(Dst.get())->VertexBufferView.SizeInBytes);
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12VertexBuffer*>(Dst.get())->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_commandList->ResourceBarrier(1, &var2);

}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIUniformBuffer> Dst, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> Src)
{
    if (Dst.empty() || Src.empty())return;
    if (static_cast<DX12UniformBuffer*>(Dst.get())->UniformBuffer.Get() == nullptr)return;
    if (static_cast<DX12UniformBuffer*>(Src.get())->UniformBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12UniformBuffer*>(Dst.get())->UniformBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    m_commandList->ResourceBarrier(1, &var1);
    m_commandList->CopyBufferRegion(static_cast<DX12UniformBuffer*>(Dst.get())->UniformBuffer.Get(), 0, static_cast<DX12UniformBuffer*>(Src.get())->UniformBuffer.Get(), 0, static_cast<DX12UniformBuffer*>(Dst.get())->GetCount()* static_cast<DX12UniformBuffer*>(Dst.get())->GetStride());
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12UniformBuffer*>(Dst.get())->UniformBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_commandList->ResourceBarrier(1, &var2);

}
void DX12Context::Lock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> UnorderedAccess)
{
    BEAR_CHECK(!UnorderedAccess.empty());
    DX12UnorderedAccess* UAV = reinterpret_cast<DX12UnorderedAccess*>(UnorderedAccess.get()->QueryInterface(DX12Q_UnorderedAccess));
    UAV->LockUAV(m_commandList.Get());
}
void DX12Context::Unlock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> UnorderedAccess)
{
    BEAR_CHECK(!UnorderedAccess.empty());
    DX12UnorderedAccess* UAV = reinterpret_cast<DX12UnorderedAccess*>(UnorderedAccess.get()->QueryInterface(DX12Q_UnorderedAccess));
    UAV->UnlockUAV(m_commandList.Get());
}