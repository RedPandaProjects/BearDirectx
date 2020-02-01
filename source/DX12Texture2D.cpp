#include "DX12PCH.h"
bsize Texture2DCounter = 0;
DX12Texture2D::DX12Texture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, BearTextureUsage typeUsage, void* data)
{
	Texture2DCounter++;
	TextureType = TT_Default;
	Format = PixelFormat;
	TextureUsage = typeUsage;
	m_buffer = 0;
	bear_fill(TextureDesc);
	TextureDesc.MipLevels = static_cast<UINT16>(Mips);
	TextureDesc.Format = Factory->Translation(PixelFormat);
	TextureDesc.Width = static_cast<uint32>(Width);
	TextureDesc.Height = static_cast<uint32>(Height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	TextureDesc.DepthOrArraySize = static_cast<UINT16>(Count);
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_DEFAULT);

	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&TextureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));
	bear_fill(DX12ShaderResource::SRV);
	DX12ShaderResource::SRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	DX12ShaderResource::SRV.Format = TextureDesc.Format;
	if (TextureDesc.DepthOrArraySize > 1)
	{
		DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		DX12ShaderResource::SRV.Texture2DArray.MipLevels = static_cast<UINT>(Mips);
		DX12ShaderResource::SRV.Texture2DArray.ArraySize = static_cast<UINT>(Count);
	}
	else
	{
		DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		DX12ShaderResource::SRV.Texture2D.MipLevels = static_cast<UINT>(Mips);
	}
	

	if (TU_STATIC!= TextureUsage)AllocBuffer();

	if (data)
	{
		auto ptr = reinterpret_cast<uint8*>(data);
		for (bsize x = 0; x < Count; x++)
			for (bsize y = 0; y < Mips; y++)
			{
				bsize  size =BearTextureUtils::GetSizeDepth(BearTextureUtils::GetMip(TextureDesc.Width, y),BearTextureUtils::GetMip(TextureDesc.Height, y), Format);
				bear_copy(Lock(y, x), ptr, size);
				Unlock();
				ptr += size;
			}


	}
}

DX12Texture2D::DX12Texture2D(bsize Width, bsize Height, BearRenderTargetFormat PixelFormat)
{
	TextureType = TT_RenderTarget;
	RTVFormat = PixelFormat;
	TextureUsage = TU_STATIC;
	m_buffer = 0;
	Texture2DCounter++;
	bear_fill(TextureDesc);
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = DX12Factory::Translation(RTVFormat);
	TextureDesc.Width = static_cast<uint32>(Width);
	TextureDesc.Height = static_cast<uint32>(Height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	TextureDesc.DepthOrArraySize = 1;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_DEFAULT);

	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&TextureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));
	bear_fill(DX12ShaderResource::SRV);
	DX12ShaderResource::SRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	DX12ShaderResource::SRV.Format = TextureDesc.Format;
	DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	DX12ShaderResource::SRV.Texture2D.MipLevels = static_cast<UINT>(1);

}

DX12Texture2D::DX12Texture2D(bsize Width, bsize Height, BearDepthStencilFormat Format)
{
	TextureType = TT_DepthStencil;
	DSVFormat = Format;
	TextureUsage = TU_STATIC;
	m_buffer = 0;
	Texture2DCounter++;
	bear_fill(TextureDesc);
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = DX12Factory::Translation(RTVFormat);
	TextureDesc.Width = static_cast<uint32>(Width);
	TextureDesc.Height = static_cast<uint32>(Height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	TextureDesc.DepthOrArraySize = 1;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_DEFAULT);

	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&TextureDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_DEPTH_READ,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));
	bear_fill(DX12ShaderResource::SRV);
}

void DX12Texture2D::SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE* HEAP)
{
	BEAR_ASSERT(TextureType != TT_DepthStencil);
	Factory->Device->CreateShaderResourceView(TextureBuffer.Get(),&(DX12ShaderResource::SRV), *HEAP);
}

DX12Texture2D::~DX12Texture2D()
{
	--Texture2DCounter;
}

void* DX12Texture2D::Lock(bsize mip, bsize depth)
{
	if(TextureBuffer.Get() == 0)return 0;
	if (TextureType != TT_Default)return 0 ;
	if (m_buffer)Unlock();
	m_mip = mip;
	m_depth = depth;

	switch (TextureUsage)
	{
	case TU_STATIC:
		AllocBuffer();
		break;
	case TU_DYNAMIC:
		break;
	case TU_STATING:
		Factory->LockCommandList();
		auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
		Factory->CommandList->ResourceBarrier(1, &var3);
		{
			D3D12_SUBRESOURCE_FOOTPRINT PitchedDesc = {  };
			PitchedDesc.Format = TextureDesc.Format;
			PitchedDesc.Width = static_cast<UINT>(BearTextureUtils::GetMip(TextureDesc.Width, m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Width = BearMath::max(UINT(4), PitchedDesc.Width);
			PitchedDesc.Height = static_cast<UINT>(BearTextureUtils::GetMip(TextureDesc.Height, m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Height = BearMath::max(UINT(4), PitchedDesc.Height);
			PitchedDesc.Depth = 1;
			PitchedDesc.RowPitch = static_cast<UINT> (BearTextureUtils::GetSizeWidth(PitchedDesc.Width, Format));



			D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedTexture2D = { 0 };
			PlacedTexture2D.Offset = 0;
			PlacedTexture2D.Footprint = PitchedDesc;

			CD3DX12_TEXTURE_COPY_LOCATION Src(TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(m_mip), static_cast<UINT>(m_depth), 0, TextureDesc.MipLevels, TextureDesc.DepthOrArraySize));
			CD3DX12_TEXTURE_COPY_LOCATION Dst(Buffer.Get(), PlacedTexture2D);
			Factory->CommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, 0);
		}
		auto var4 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Factory->CommandList->ResourceBarrier(1, &var4);
		Factory->UnlockCommandList();
		break;
	default:
		break;
	}

	
	CD3DX12_RANGE readRange(0, 0);
	R_CHK(Buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer)));
	return m_buffer;
}

void DX12Texture2D::Unlock()
{
	if (TextureBuffer.Get() == 0)
		return;
	if (m_buffer == 0)return;
	Buffer->Unmap(0, nullptr);

	switch (TextureUsage)
	{
	case TU_STATIC:
	case TU_DYNAMIC:
		Factory->LockCommandList();
		auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &var3);
		{
			D3D12_SUBRESOURCE_FOOTPRINT PitchedDesc = {  };
			PitchedDesc.Format = TextureDesc.Format;
			PitchedDesc.Width = static_cast<UINT>(BearTextureUtils::GetMip(TextureDesc.Width, m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Width = BearMath::max(UINT(4), PitchedDesc.Width);
			PitchedDesc.Height = static_cast<UINT>(BearTextureUtils::GetMip(TextureDesc.Height, m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Height = BearMath::max(UINT(4), PitchedDesc.Height);
			PitchedDesc.Depth = 1;
			PitchedDesc.RowPitch = static_cast<UINT> (BearTextureUtils::GetSizeWidth(PitchedDesc.Width, Format));
			bsize Delta = ((PitchedDesc.RowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)) - PitchedDesc.RowPitch;
			PitchedDesc.RowPitch = (PitchedDesc.RowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);

			if (Delta)
			{
				auto dst = m_buffer;
				auto src = m_buffer;
				dst += PitchedDesc.RowPitch * (BearTextureUtils::GetCountBlock(PitchedDesc.Height, Format) - 1);
				src += (PitchedDesc.RowPitch - Delta) * (BearTextureUtils::GetCountBlock(PitchedDesc.Height, Format) - 1);
				for (bint i = BearTextureUtils::GetCountBlock(PitchedDesc.Height, Format); i > 0; i--)
				{
					bear_move(dst, src, PitchedDesc.RowPitch - Delta);
					dst -= PitchedDesc.RowPitch;
					src -= (PitchedDesc.RowPitch - Delta);
				}
			}

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedTexture2D = { 0 };
			PlacedTexture2D.Offset = 0;
			PlacedTexture2D.Footprint = PitchedDesc;

			CD3DX12_TEXTURE_COPY_LOCATION Dst(TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(m_mip), static_cast<UINT>(m_depth), 0, TextureDesc.MipLevels, TextureDesc.DepthOrArraySize));
			CD3DX12_TEXTURE_COPY_LOCATION Src(Buffer.Get(), PlacedTexture2D);
			Factory->CommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, 0);
		}
		auto var4 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Factory->CommandList->ResourceBarrier(1, &var4);
		Factory->UnlockCommandList();
		break;
	case TU_STATING:
		break;
	default:
		break;
	}

	
	if (TextureUsage == TU_STATIC)
	{
		FreeBuffer();
	}

	m_buffer = 0;


}

BearTextureType DX12Texture2D::GetType()
{
	return TextureType;
}

void DX12Texture2D::AllocBuffer()
{
	bsize SizeWidth = 0;
	bsize SizeDepth = 0;
	{
		SizeWidth = (BearTextureUtils::GetSizeWidth(TextureDesc.Width, Format));
		SizeWidth = (SizeWidth + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
		SizeDepth = SizeWidth * BearTextureUtils::GetCountBlock(TextureDesc.Height, Format);
		SizeDepth = (SizeDepth + D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);
	}
	CD3DX12_HEAP_PROPERTIES var1(TU_STATING == TextureUsage ? D3D12_HEAP_TYPE_READBACK : D3D12_HEAP_TYPE_UPLOAD);
	auto var2 = CD3DX12_RESOURCE_DESC::Buffer(SizeDepth);
	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&var2,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Buffer)));
}

void DX12Texture2D::FreeBuffer()
{
	Buffer.Reset();
}
