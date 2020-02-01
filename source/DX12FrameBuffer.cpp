#include "DX12PCH.h"
bsize FrameBufferCounter = 0;
DX12FrameBuffer::DX12FrameBuffer(const BearFrameBufferDescription& description) :Description(description)
{
	FrameBufferCounter++;
	BEAR_ASSERT(!description.RenderPass.empty());
	RenderPassRef = static_cast<DX12RenderPass*>(Description.RenderPass.get());

	for (Count = 0; Count < 8; Count++)
	{
		if (Description.RenderTargets[Count].empty())
		{
			break;
		}
		BEAR_ASSERT(Description.RenderTargets[Count]->GetType() == TT_RenderTarget);
		auto texture = static_cast<DX12Texture2D*>(Description.RenderTargets[Count].get());
		BEAR_ASSERT(texture->RTVFormat == RenderPassRef->Description.RenderTargets[Count].Format);

	}

	BEAR_ASSERT(RenderPassRef->CountRenderTarget == Count);
	{
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = static_cast<UINT>( Count);
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			R_CHK(Factory->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RtvHeap)));

		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (bsize i = 0; Count > i; i++)
		{
			D3D12_RENDER_TARGET_VIEW_DESC DESC = {};


			auto texture = static_cast<DX12Texture2D*>(Description.RenderTargets[i].get());
			DESC.Format = texture->TextureDesc.Format;
			DESC.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			Factory->Device->CreateRenderTargetView(texture->TextureBuffer.Get(), &DESC, rtvHandle);

			rtvHandle.Offset(1, Factory->RtvDescriptorSize);
		}
	}

	if (Description.DepthStencil.empty())
	{
		BEAR_ASSERT(RenderPassRef->Description.DepthStencil.Format == DSF_NONE);
	}
	else
	{
		BEAR_ASSERT(Description.DepthStencil.get()->GetType() == TT_DepthStencil);
		BEAR_ASSERT(RenderPassRef->Description.DepthStencil.Format == static_cast<DX12Texture2D*>(Description.DepthStencil.get())->DSVFormat);
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = 1;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			R_CHK(Factory->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&DsvHeap)));

		}

		{
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DsvHeap->GetCPUDescriptorHandleForHeapStart());
		D3D12_DEPTH_STENCIL_VIEW_DESC DESC = {};
		DESC.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DESC.Format = static_cast<DX12Texture2D*>(Description.DepthStencil.get())->TextureDesc.Format;
		Factory->Device->CreateDepthStencilView(static_cast<DX12Texture2D*>(Description.DepthStencil.get())->TextureBuffer.Get(), &DESC, rtvHandle);
		DepthStencilRef = rtvHandle;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (bsize i = 0; Count > i; i++)
	{
		RenderTargetRefs[i] = rtvHandle;
		rtvHandle.Offset(1, Factory->RtvDescriptorSize);
	}


}

DX12FrameBuffer::~DX12FrameBuffer()
{
	FrameBufferCounter--;

}

void DX12FrameBuffer::ToTexture(ID3D12GraphicsCommandList* Cmd)
{
	for (bsize i = 0; Count > i; i++)
	{
		auto texture = static_cast<DX12Texture2D*>(Description.RenderTargets[i].get());

		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture->TextureBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Cmd->ResourceBarrier(1, &transition);
	}
}

void DX12FrameBuffer::ToRT(ID3D12GraphicsCommandList* Cmd)
{
	for (bsize i = 0; Count > i; i++)
	{
		auto texture = static_cast<DX12Texture2D*>(Description.RenderTargets[i].get());
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture->TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		Cmd->ResourceBarrier(1, &transition);
	}
}
