#include "DX12PCH.h"

DX12RenderTexture2DUAV::DX12RenderTexture2DUAV()
{
}

void DX12RenderTexture2DUAV::Create(bsize width, bsize height, bsize depth, BearGraphics::BearTextureUAVPixelFormat format)
{
	Clear();
	bear_fill(TextureDesc);
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = Factory->Translation(format);
	TextureDesc.Width = static_cast<uint32>(width);
	TextureDesc.Height = static_cast<uint32>(height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	TextureDesc.DepthOrArraySize = static_cast<UINT16>(depth);
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_DEFAULT);

	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&TextureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));

	{
		bear_fill(TextureView);
		TextureView.Format = TextureDesc.Format;
		if (TextureDesc.DepthOrArraySize > 1)
		{
			TextureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			TextureView.Texture2DArray.ArraySize = static_cast<UINT>(depth);
		}
		else
		{
			TextureView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		}
	}
	{
		bear_fill(TextureViewForShader);
		TextureViewForShader.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		TextureViewForShader.Format = TextureDesc.Format;
		if (TextureDesc.DepthOrArraySize > 1)
		{
			TextureViewForShader.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			TextureViewForShader.Texture2DArray.ArraySize = static_cast<UINT>(depth);
			TextureViewForShader.Texture2DArray.MipLevels = 1;
		}
		else
		{
			TextureViewForShader.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			TextureViewForShader.Texture2D.MipLevels = 1;
		}
	}
	
}

void DX12RenderTexture2DUAV::Clear()
{
	TextureBuffer.Reset();
}

DX12RenderTexture2DUAV::~DX12RenderTexture2DUAV()
{
	Clear();
}



void DX12RenderTexture2DUAV::SetResource(void * heap)
{
	D3D12_CPU_DESCRIPTOR_HANDLE* Handle = reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(heap);
	Factory->Device->CreateShaderResourceView(TextureBuffer.Get(), &TextureViewForShader, *Handle);
}

void DX12RenderTexture2DUAV::SetUnorderedAccess(void *heap)
{
	D3D12_CPU_DESCRIPTOR_HANDLE* Handle = reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(heap);
	Factory->Device->CreateUnorderedAccessView(TextureBuffer.Get(), nullptr, &TextureView, *Handle);
}



void DX12RenderTexture2DUAV::ToResource(ID3D12GraphicsCommandList * CommandList)
{
	auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandList->ResourceBarrier(1, &var3);
}

void DX12RenderTexture2DUAV::ToUnordered(ID3D12GraphicsCommandList * CommandList)
{
	auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	CommandList->ResourceBarrier(1, &var3);
}

