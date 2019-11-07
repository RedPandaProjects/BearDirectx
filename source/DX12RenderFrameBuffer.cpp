#include "DX12PCH.h"

DX12RenderFrameBuffer::DX12RenderFrameBuffer(const BearGraphics::BearRenderFrameBufferDescription & desc):Description(desc)
{
	for (RTCount = 0; RTCount < 8&&!desc.RenderTargets[RTCount].RenderTargetBuffer.empty(); RTCount++)
	{
		const DX12RenderTargetView*RT = static_cast<const DX12RenderTargetView*>(desc.RenderTargets[RTCount].RenderTargetBuffer.get());
		RTBufferPointer[RTCount] =const_cast<DX12RenderTargetView*> (RT);
		RTPointer[RTCount] = RT->Heap->GetCPUDescriptorHandleForHeapStart();
	}
}

DX12RenderFrameBuffer::~DX12RenderFrameBuffer()
{
}

void DX12RenderFrameBuffer::Clear(ID3D12GraphicsCommandList * CommandLine)
{
	for (bsize i = 0; i < RTCount; i++)
		if (Description.RenderTargets[i].NeedClear)
			CommandLine->ClearRenderTargetView(RTPointer[i], Description.RenderTargets[i].ClearColor.GetFloat().array,0,0);
}

void DX12RenderFrameBuffer::Set(ID3D12GraphicsCommandList * CommandLine)
{
	CommandLine->OMSetRenderTargets(static_cast<UINT>(RTCount),RTPointer,0,0);
}

void DX12RenderFrameBuffer::ToPresent(ID3D12GraphicsCommandList * CommandLine)
{
	for (bsize i = 0; i < RTCount; i++)
	{
		auto var = CD3DX12_RESOURCE_BARRIER::Transition(RTBufferPointer[i]->RTTexture.TextureBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		CommandLine->ResourceBarrier(1, &var);
	}
}

void DX12RenderFrameBuffer::ToRT(ID3D12GraphicsCommandList * CommandLine)
{
	for (bsize i = 0; i < RTCount; i++)
	{
		auto var = CD3DX12_RESOURCE_BARRIER::Transition(RTBufferPointer[i]->RTTexture.TextureBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT,  D3D12_RESOURCE_STATE_RENDER_TARGET);
		CommandLine->ResourceBarrier(1, &var);
	}
}
