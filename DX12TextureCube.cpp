#include "DX12PCH.h"

#pragma optimize("", off)
bsize TextureCubeCounter = 0;
DX12TextureCube::DX12TextureCube(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, BearTextureUsage typeUsage, void* data)
{
	TextureCubeCounter++;
	Format = PixelFormat;
	TextureUsage = typeUsage;
	m_buffer = 0;
	ZeroMemory(&TextureDesc,sizeof(TextureDesc));
	TextureDesc.MipLevels = static_cast<UINT16>(Mips);
	TextureDesc.Format = Factory->Translation(PixelFormat);
	TextureDesc.Width = static_cast<uint32>(Width);
	TextureDesc.Height = static_cast<uint32>(Height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	TextureDesc.DepthOrArraySize = static_cast<UINT16>(Count)*6;
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
	ZeroMemory(&(DX12ShaderResource::SRV), sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	DX12ShaderResource::SRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	DX12ShaderResource::SRV.Format = TextureDesc.Format;
	if (TextureDesc.DepthOrArraySize > 6)
	{
		DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
		DX12ShaderResource::SRV.TextureCubeArray.MipLevels = static_cast<UINT>(Mips);
		DX12ShaderResource::SRV.TextureCubeArray.NumCubes = static_cast<UINT>(Count)/6;
	}
	else
	{
		DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		DX12ShaderResource::SRV.TextureCube.MipLevels = static_cast<UINT>(Mips);
	}
	

	if (TU_STATIC!= TextureUsage)AllocBuffer();

	if (data)
	{
		auto ptr = reinterpret_cast<uint8*>(data);
		for (bsize x = 0; x < Count*6; x++)
			for (bsize y = 0; y < Mips; y++)
			{
				bsize  size =BearTextureUtils::GetSizeDepth(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), y),BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), y), Format);
				memcpy(Lock(y, x), ptr, size);
				Unlock();
				ptr += size;
			}


	}
}

bool DX12TextureCube::SetAsSRV(D3D12_CPU_DESCRIPTOR_HANDLE& HEAP)
{
	Factory->Device->CreateShaderResourceView(TextureBuffer.Get(),&(DX12ShaderResource::SRV), HEAP);
	return true;
}

void* DX12TextureCube::QueryInterface(int Type)
{
	switch (Type)
	{
	case DX12Q_ShaderResource:
		return reinterpret_cast<void*>(static_cast<DX12ShaderResource*>(this));
	default:
		return nullptr;
	}
}

DX12TextureCube::~DX12TextureCube()
{
	--TextureCubeCounter;
}

void* DX12TextureCube::Lock(bsize mip, bsize depth)
{
	if(TextureBuffer.Get() == 0)return 0;
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
			PitchedDesc.Width = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Width = BearMath::max(UINT(4), PitchedDesc.Width);
			PitchedDesc.Height = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), m_mip));
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

void DX12TextureCube::Unlock()
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
			PitchedDesc.Width = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Width = BearMath::max(UINT(4), PitchedDesc.Width);
			PitchedDesc.Height = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), m_mip));
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
				for (bsize i = BearTextureUtils::GetCountBlock(PitchedDesc.Height, Format); i > 0; i--)
				{
					memmove(dst, src, PitchedDesc.RowPitch - Delta);
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


void DX12TextureCube::AllocBuffer()
{
	bsize SizeWidth = 0;
	bsize SizeDepth = 0;
	{
		SizeWidth = (BearTextureUtils::GetSizeWidth(static_cast<bsize>(TextureDesc.Width), Format));
		SizeWidth = (SizeWidth + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
		SizeDepth = SizeWidth * BearTextureUtils::GetCountBlock(static_cast<bsize>(TextureDesc.Height), Format);
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

void DX12TextureCube::FreeBuffer()
{
	Buffer.Reset();
}
