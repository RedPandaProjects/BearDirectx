#include "DX12PCH.h"

DX12RenderTexture2D::DX12RenderTexture2D()
{
}

void DX12RenderTexture2D::Create(bsize width, bsize height, bsize mips, bsize depth, BearGraphics::BearTexturePixelFormat format, void * data, bool dynamic)
{
	Clear();
	m_format = format;
	bear_fill(TextureDesc);
	TextureDesc.MipLevels = static_cast<UINT16>(mips);
	TextureDesc.Format = Factory->Translation(format);
	TextureDesc.Width =static_cast<uint32>( width);
	TextureDesc.Height = static_cast<uint32>(height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	TextureDesc.DepthOrArraySize =static_cast<UINT16>( depth);
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_DEFAULT);

	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&TextureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));

	bear_fill(TextureView);
	TextureView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	TextureView.Format = TextureDesc.Format;
	if (TextureDesc.DepthOrArraySize > 1)
	{
		TextureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		TextureView.Texture2DArray.MipLevels = static_cast<UINT>(mips);
		TextureView.Texture2DArray.ArraySize = static_cast<UINT>(depth);
	}
	else
	{
		TextureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		TextureView.Texture2D.MipLevels = static_cast<UINT>(mips);
	}


	if (data)
	{
		auto ptr = reinterpret_cast<uint8*>(data);
		for(bsize x=0;x<depth;x++)
			for (bsize y = 0; y < mips; y++)
			{
				bsize  size = BearGraphics::BearTextureUtils::GetSizeDepth(BearGraphics::BearTextureUtils::GetMip(TextureDesc.Width, y), BearGraphics::BearTextureUtils::GetMip(TextureDesc.Height, y), m_format);
				bear_copy(Lock(y,x), ptr, size);
				ptr += size;
			}
	
		Unlock();
	}
}

void * DX12RenderTexture2D::Lock(bsize mip, bsize depth)
{
	if (TextureBuffer.Get() == 0)
		return 0;
	if (m_dynamic)
	{
		BEAR_ASSERT(false);
		return 0;
	}
	else
	{
		m_mip = mip;
		m_depth= depth;
		m_buffer = bear_alloc<uint8>(BearGraphics::BearTextureUtils::GetSizeDepth(BearGraphics::BearTextureUtils::GetMip(TextureDesc.Width, mip), BearGraphics::BearTextureUtils::GetMip(TextureDesc.Height, mip),m_format));
		return m_buffer;
	}
	
}

void DX12RenderTexture2D::Unlock()
{
	if (TextureBuffer.Get() == 0)
		return ;
	if (m_dynamic)
	{
		BEAR_ASSERT(false);
	}
	else
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
		ResourceData.RowPitch = BearGraphics::BearTextureUtils::GetSizeWidth(TextureDesc.Width, m_format);
		ResourceData.SlicePitch = ResourceData.RowPitch*TextureDesc.Height;
		Factory->LockCopyCommandList();
		UpdateSubresources<1>(Factory->CopyCommandList.Get(), TextureBuffer.Get(), UploadHeap.Get(), 0, D3D12CalcSubresource(static_cast<UINT>(m_mip), static_cast<UINT>(m_depth),0,TextureDesc.MipLevels, TextureDesc.DepthOrArraySize), 1, &ResourceData);
		Factory->UnlockCopyCommandList();
		bear_free(m_buffer); m_buffer = 0;
		//actory->CommandList->CopyTextureRegion()
		Factory->LockCommandList();
		auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		Factory->CommandList->ResourceBarrier(1, &var3);
		Factory->UnlockCommandList();
	}
}

void DX12RenderTexture2D::Clear()
{
	if (m_buffer)bear_free(m_buffer);
	m_buffer = 0;
	TextureBuffer.Reset();
	m_dynamic = false;
	m_format = BearGraphics::TPF_NONE;
}

DX12RenderTexture2D::~DX12RenderTexture2D()
{
	Clear();
}

void * DX12RenderTexture2D::GetHandle()
{
	return &TextureView;
}
