#include "DX12PCH.h"
bsize Texture2DCounter = 0;
DX12Texture2D::DX12Texture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, BearTextureUsage typeUsage, void* data, bool UAV )
{
	Texture2DCounter++;
	bAllowUAV = UAV;
	TextureType = TT_Default;
	Format = PixelFormat;
	TextureUsage = typeUsage;
	m_buffer = 0;
	ZeroMemory(&TextureDesc,sizeof(TextureDesc));
	TextureDesc.MipLevels = static_cast<UINT16>(Mips);
	TextureDesc.Format = Factory->Translation(PixelFormat);
	TextureDesc.Width = static_cast<uint32>(Width);
	TextureDesc.Height = static_cast<uint32>(Height);
	TextureDesc.Flags =UAV? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS:D3D12_RESOURCE_FLAG_NONE;
	TextureDesc.DepthOrArraySize = static_cast<UINT16>(Count);
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_DEFAULT);
	CurrentStates = bAllowUAV ? (D3D12_RESOURCE_STATE_UNORDERED_ACCESS) : (CurrentStates);
	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&TextureDesc,
		CurrentStates,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));
	ZeroMemory(&(DX12ShaderResource::SRV), sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));

	{
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
	}
	if(UAV)
	{
		ZeroMemory(&(DX12UnorderedAccess::UAV), sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));
		DX12UnorderedAccess::UAV.Format = TextureDesc.Format;
		if (TextureDesc.DepthOrArraySize > 1)
		{
			DX12UnorderedAccess::UAV.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			DX12UnorderedAccess::UAV.Texture2DArray.ArraySize = static_cast<UINT>(Count);
		}
		else
		{
			DX12UnorderedAccess::UAV.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		}
	}
	

	if (TU_STATIC!= TextureUsage)AllocBuffer();

	if (data)
	{
		auto ptr = reinterpret_cast<uint8*>(data);
		for (bsize x = 0; x < Count; x++)
			for (bsize y = 0; y < Mips; y++)
			{
				bsize  size =BearTextureUtils::GetSizeDepth(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), y),BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), y), Format);
				memcpy(Lock(y, x), ptr, size);
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
	ZeroMemory(&TextureDesc, sizeof(TextureDesc));
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
		CurrentStates,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));
	ZeroMemory(&(DX12ShaderResource::SRV), sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
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
	ZeroMemory(&TextureDesc, sizeof(TextureDesc));
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = DX12Factory::Translation(DSVFormat);
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
		D3D12_RESOURCE_STATE_DEPTH_WRITE ,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));
	ZeroMemory(&(DX12ShaderResource::SRV), sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
}

bool DX12Texture2D::SetAsSRV(D3D12_CPU_DESCRIPTOR_HANDLE& HEAP)
{
	BEAR_CHECK(TextureType != TT_DepthStencil);
	Factory->Device->CreateShaderResourceView(TextureBuffer.Get(),&(DX12ShaderResource::SRV), HEAP);
	return true;
}

bool DX12Texture2D::SetAsUAV(D3D12_CPU_DESCRIPTOR_HANDLE& HEAP, bsize offset)
{
	if (!bAllowUAV)return false;
	BEAR_CHECK(TextureType != TT_DepthStencil);
	DX12UnorderedAccess::UAV.Texture2D.MipSlice = offset;
	DX12UnorderedAccess::UAV.Texture2DArray.MipSlice = offset;
	Factory->Device->CreateUnorderedAccessView(TextureBuffer.Get(),nullptr, &(DX12UnorderedAccess::UAV), HEAP);
	return true;
}

void* DX12Texture2D::QueryInterface(int Type)
{
	switch (Type)
	{
	case DX12Q_ShaderResource:
		return reinterpret_cast<void*>(static_cast<DX12ShaderResource*>(this));
	case DX12Q_UnorderedAccess:
		if (!bAllowUAV)return nullptr;
		return reinterpret_cast<void*>(static_cast<DX12UnorderedAccess*>(this));
	default:
		return nullptr;
	}
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
		auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), CurrentStates, D3D12_RESOURCE_STATE_COPY_SOURCE);
		Factory->CommandList->ResourceBarrier(1, &var3);
		{
			D3D12_SUBRESOURCE_FOOTPRINT PitchedDesc = {  };
			PitchedDesc.Format = TextureDesc.Format;
			PitchedDesc.Width = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Width =BearMath::max(UINT(4), PitchedDesc.Width);
			PitchedDesc.Height = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Height =BearMath::max(UINT(4), PitchedDesc.Height);
			PitchedDesc.Depth = 1;
			PitchedDesc.RowPitch = static_cast<UINT> (BearTextureUtils::GetSizeWidth(PitchedDesc.Width, Format));



			D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedTexture2D = { 0 };
			PlacedTexture2D.Offset = 0;
			PlacedTexture2D.Footprint = PitchedDesc;

			CD3DX12_TEXTURE_COPY_LOCATION Src(TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(m_mip), static_cast<UINT>(m_depth), 0, TextureDesc.MipLevels, TextureDesc.DepthOrArraySize));
			CD3DX12_TEXTURE_COPY_LOCATION Dst(Buffer.Get(), PlacedTexture2D);
			Factory->CommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, 0);
		}
		auto var4 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, CurrentStates);
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
		auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), CurrentStates, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &var3);
		{
			D3D12_SUBRESOURCE_FOOTPRINT PitchedDesc = {  };
			PitchedDesc.Format = TextureDesc.Format;
			PitchedDesc.Width = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Width =BearMath::max(UINT(4), PitchedDesc.Width);
			PitchedDesc.Height = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), m_mip));
			if (BearTextureUtils::isCompressor(Format))
				PitchedDesc.Height =BearMath::max(UINT(4), PitchedDesc.Height);
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
		auto var4 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, CurrentStates);
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

void DX12Texture2D::FreeBuffer()
{
	Buffer.Reset();
}

void DX12Texture2D::LockUAV(
#if defined(DX12) | defined(DX12_1)
	ComPtr<ID3D12GraphicsCommandList4> CommandList
#else
	ComPtr<ID3D12GraphicsCommandList> CommandList
#endif
)
{
	BEAR_CHECK(bAllowUAV);
	auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), CurrentStates, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	CommandList->ResourceBarrier(1, &var1);
}

void DX12Texture2D::UnlockUAV(
#if defined(DX12) | defined(DX12_1)
	ComPtr<ID3D12GraphicsCommandList4> CommandList
#else
	ComPtr<ID3D12GraphicsCommandList> CommandList
#endif
)
{
	BEAR_CHECK(bAllowUAV);
	auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, CurrentStates);
	CommandList->ResourceBarrier(1, &var1);
}
