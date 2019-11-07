#include "DX12PCH.h"

DX12RenderTargetView::DX12RenderTargetView(const BearGraphics::BearRenderTargetViewDescription&Description)
{
	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.NumDescriptors = 1;
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		R_CHK(Factory->Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&Heap)));
	}
	RTTexture.Create(Description.Width, Description.Height, Description.Format);
	Format = Description.Format;
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format = Factory->Translation(Description.Format);
	desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
	Factory->Device->CreateRenderTargetView(RTTexture.TextureBuffer.Get(), &desc, Heap->GetCPUDescriptorHandleForHeapStart());
}

DX12RenderTargetView::~DX12RenderTargetView()
{

}

void DX12RenderTargetView::Resize(bsize Width, bsize Height)
{
	RTTexture.Create(Width, Height, Format);
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format = Factory->Translation(Format);
	desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
	Factory->Device->CreateRenderTargetView(RTTexture.TextureBuffer.Get(), &desc, Heap->GetCPUDescriptorHandleForHeapStart());

}

void DX12RenderTargetView::SetResource(void * heap)
{
	RTTexture.SetResource(heap);
}


