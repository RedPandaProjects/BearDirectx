#include "DX12PCH.h"

DX12Texture2D::DX12Texture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, void* data)
{
	m_format = PixelFormat;
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
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
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
	

	if (data)
	{
		ComPtr<ID3D12Resource> UploadBuffer;

		{
			UINT64 SizeWidth = 0;
			UINT64 SizeDepth = 0;
			{
				SizeWidth = (BearTextureUtils::GetSizeWidth(static_cast<bsize>(TextureDesc.Width), m_format));
				SizeWidth = (SizeWidth + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
				SizeDepth = SizeWidth * BearTextureUtils::GetCountBlock(static_cast<bsize>(TextureDesc.Height), m_format);
				SizeDepth = (SizeDepth + D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);
			}
			CD3DX12_HEAP_PROPERTIES var3(D3D12_HEAP_TYPE_UPLOAD);
			auto var2 = CD3DX12_RESOURCE_DESC::Buffer(SizeDepth);
			R_CHK(Factory->Device->CreateCommittedResource(
				&var3,
				D3D12_HEAP_FLAG_NONE,
				&var2,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&UploadBuffer)));
		}


		auto ptr = reinterpret_cast<uint8*>(data);
		for (bsize x = 0; x < Count; x++)
			for (bsize y = 0; y < Mips; y++)
			{
				uint8* buffer;
				{
					CD3DX12_RANGE readRange(0, 0);
					R_CHK(UploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&buffer)));
				}
				bsize  size = BearTextureUtils::GetSizeDepth(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), y), BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), y), m_format);
				bear_copy(buffer, ptr, size);
				{
					UploadBuffer->Unmap(0, nullptr);
					Factory->LockCommandList();
					{
						auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
						Factory->CommandList->ResourceBarrier(1, &var3);
					}
					{
						D3D12_SUBRESOURCE_FOOTPRINT PitchedDesc = {  };
						PitchedDesc.Format = TextureDesc.Format;
						PitchedDesc.Width = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), y));
						if (BearTextureUtils::isCompressor(m_format))
							PitchedDesc.Width = BearMath::max(UINT(4), PitchedDesc.Width);
						PitchedDesc.Height = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), y));
						if (BearTextureUtils::isCompressor(m_format))
							PitchedDesc.Height = BearMath::max(UINT(4), PitchedDesc.Height);
						PitchedDesc.Depth = 1;
						PitchedDesc.RowPitch = static_cast<UINT> (BearTextureUtils::GetSizeWidth(PitchedDesc.Width, m_format));
						bsize Delta = ((PitchedDesc.RowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)) - PitchedDesc.RowPitch;
						PitchedDesc.RowPitch = (PitchedDesc.RowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);

						if (Delta)
						{
							auto dst = buffer;
							auto src = buffer;
							dst += PitchedDesc.RowPitch * (BearTextureUtils::GetCountBlock(PitchedDesc.Height, m_format) - 1);
							src += (PitchedDesc.RowPitch - Delta) * (BearTextureUtils::GetCountBlock(PitchedDesc.Height, m_format) - 1);
							for (bint i = BearTextureUtils::GetCountBlock(PitchedDesc.Height, m_format); i >= 0; i--)
							{
								bear_move(dst, src, PitchedDesc.RowPitch - Delta);
								dst -= PitchedDesc.RowPitch;
								src -= (PitchedDesc.RowPitch - Delta);
							}
						}

						D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedTexture2D = { 0 };
						PlacedTexture2D.Offset = 0;
						PlacedTexture2D.Footprint = PitchedDesc;

						CD3DX12_TEXTURE_COPY_LOCATION Dst(TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(y), static_cast<UINT>(x), 0, TextureDesc.MipLevels, TextureDesc.DepthOrArraySize));
						CD3DX12_TEXTURE_COPY_LOCATION Src(UploadBuffer.Get(), PlacedTexture2D);
						Factory->CommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, 0);
					}
					
					{
						auto var4 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
						Factory->CommandList->ResourceBarrier(1, &var4);
						Factory->UnlockCommandList();
					}
				}
				ptr += size;
			}


	}
}

void DX12Texture2D::SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE* HEAP)
{
	Factory->Device->CreateShaderResourceView(TextureBuffer.Get(),&(DX12ShaderResource::SRV), *HEAP);
}

DX12Texture2D::~DX12Texture2D()
{
}
