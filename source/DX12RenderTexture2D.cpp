#include "DX12PCH.h"

void DX12RenderTexture2D::Create(bsize width, bsize height, bsize mip, BearGraphics::BearTexturePixelFormat format, void * data, bool dynamic)
{
	bear_fill(textureDesc);
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	R_CHK(Factory->Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));

	m_size = GetRequiredIntermediateSize(TextureBuffer.Get(), 0, 1);
	bear_fill(TextureView);
	TextureView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	TextureView.Format = textureDesc.Format;
	TextureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	TextureView.Texture2D.MipLevels = 1;
	if (data)
	{
		bear_copy(Lock(), data, count*stride);
		Unlock();
	}
}

void * DX12RenderTexture2D::Lock()
{
	return nullptr;
}

void DX12RenderTexture2D::Unlock()
{
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(TextureBuffer.Get(), 0, 1);
	ComPtr<ID3D12Resource> UploadHeap;
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_UPLOAD);
	auto var2 = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&var2,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&UploadHeap)));

	D3D12_SUBRESOURCE_DATA ResourceData = {};
	ResourceData.pData = m_buffer;
	ResourceData.RowPitch = textureDesc.Width*4;
	ResourceData.SlicePitch = ResourceData.RowPitch*textureDesc.Height;

	Factory->LockCopyCommandList();
	UpdateSubresources<1>(Factory->CopyCommandList.Get(), TextureBuffer.Get(), UploadHeap.Get(), 0, 0, 1, &ResourceData);
	Factory->UnlockCopyCommandList();
	bear_free(m_buffer); m_buffer = 0;

	Factory->LockCommandList();
	auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	Factory->CommandList->ResourceBarrier(1, &var3);
	Factory->UnlockCommandList();
}

void DX12RenderTexture2D::Clear()
{
}

DX12RenderTexture2D::~DX12RenderTexture2D()
{
}

void * DX12RenderTexture2D::GetHandle()
{
	return &TextureView;
}
