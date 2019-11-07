#include "DX12PCH.h"

DX12RenderTexture2D::DX12RenderTexture2D()
{
	m_buffer = 0;
}

void DX12RenderTexture2D::Create(bsize width, bsize height, bsize mips, bsize depth, BearGraphics::BearTexturePixelFormat format, void * data, bool dynamic)
{
	Clear();
	m_format = format;
	m_dynamic = dynamic;
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
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
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
	if (m_dynamic)AllocUploadBuffer();

	if (data)
	{
		auto ptr = reinterpret_cast<uint8*>(data);
		for(bsize x=0;x<depth;x++)
			for (bsize y = 0; y < mips; y++)
			{
				bsize  size = BearGraphics::BearTextureUtils::GetSizeDepth(BearGraphics::BearTextureUtils::GetMip(TextureDesc.Width, y), BearGraphics::BearTextureUtils::GetMip(TextureDesc.Height, y), m_format);
				bear_copy(Lock(y,x), ptr, size);
				Unlock();
				ptr += size;
			}
	

	}
}

void DX12RenderTexture2D::Create(bsize width, bsize height, BearGraphics::BearRenderTargetFormat format)
{
	Clear();
	m_format = BearGraphics::TPF_NONE;
	m_dynamic = false;
	bear_fill(TextureDesc);
	TextureDesc.MipLevels = static_cast<UINT16>(1);
	TextureDesc.Format = Factory->Translation(format);
	TextureDesc.Width = static_cast<uint32>(width);
	TextureDesc.Height = static_cast<uint32>(height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	TextureDesc.DepthOrArraySize = static_cast<UINT16>(1);
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = TextureDesc.Format;
	bear_copy(ClearValue.Color, BearColor::Black.GetFloat().array,4);
	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&TextureDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET ,
		nullptr,
		IID_PPV_ARGS(&TextureBuffer)));

	bear_fill(TextureView);
	TextureView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	TextureView.Format = TextureDesc.Format;
	if (TextureDesc.DepthOrArraySize > 1)
	{
		TextureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		TextureView.Texture2DArray.MipLevels = static_cast<UINT>(1);
		TextureView.Texture2DArray.ArraySize = static_cast<UINT>(1);
	}
	else
	{
		TextureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		TextureView.Texture2D.MipLevels = static_cast<UINT>(1);
	}
}

void * DX12RenderTexture2D::Lock(bsize mip, bsize depth)
{
	if (TextureBuffer.Get() == 0)
		return 0;
	if (m_buffer)Unlock();
	if (!m_dynamic)
	{
		AllocUploadBuffer();
	}
	m_mip = mip;
	m_depth = depth;
	CD3DX12_RANGE readRange(0, 0);
	R_CHK(UploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer)));
	return m_buffer;
}

void DX12RenderTexture2D::Unlock()
{
	if (TextureBuffer.Get() == 0)
		return;
	if (m_buffer == 0)return;
	UploadBuffer->Unmap(0, nullptr);
	Factory->LockCommandList();
	auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	Factory->CommandList->ResourceBarrier(1, &var3);
	{
			D3D12_SUBRESOURCE_FOOTPRINT PitchedDesc = {  };
			PitchedDesc.Format = TextureDesc.Format;
			PitchedDesc.Width = static_cast<UINT>(BearGraphics::BearTextureUtils::GetMip(TextureDesc.Width, m_mip));
			if (BearGraphics::BearTextureUtils::isCompressor(m_format))
				PitchedDesc.Width = bear_max(UINT(4), PitchedDesc.Width);
			PitchedDesc.Height = static_cast<UINT>(BearGraphics::BearTextureUtils::GetMip(TextureDesc.Height, m_mip));
			if (BearGraphics::BearTextureUtils::isCompressor(m_format))
				PitchedDesc.Height = bear_max(UINT(4), PitchedDesc.Height);
			PitchedDesc.Depth = 1;
			PitchedDesc.RowPitch = static_cast<UINT> (BearGraphics::BearTextureUtils::GetSizeWidth(PitchedDesc.Width, m_format));
			bsize Delta = ((PitchedDesc.RowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT -1) &~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT-1) )- PitchedDesc.RowPitch;
			PitchedDesc.RowPitch = (PitchedDesc.RowPitch+ D3D12_TEXTURE_DATA_PITCH_ALIGNMENT-1) &~ (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT-1);

			if (Delta)
			{
				auto dst = m_buffer;
				auto src = m_buffer;
				dst += PitchedDesc.RowPitch*(BearGraphics::BearTextureUtils::GetCountBlock(PitchedDesc.Height, m_format) -1);
				src += (PitchedDesc.RowPitch- Delta)*(BearGraphics::BearTextureUtils::GetCountBlock(PitchedDesc.Height, m_format) - 1);
				for (bint i = BearGraphics::BearTextureUtils::GetCountBlock(PitchedDesc.Height, m_format); i >= 0; i--)
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
		CD3DX12_TEXTURE_COPY_LOCATION Src(UploadBuffer.Get(), PlacedTexture2D);
		Factory->CommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, 0);
	}
	auto var4 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	Factory->CommandList->ResourceBarrier(1, &var4);
	Factory->UnlockCommandList();

	m_buffer = 0;

	if (!m_dynamic)
	{
		FreeUploadBuffer();
	}
}

void DX12RenderTexture2D::Clear()
{
	if (m_buffer)Unlock();
	m_buffer = 0;
	TextureBuffer.Reset();
	UploadBuffer.Reset();
	m_dynamic = false;
	m_format = BearGraphics::TPF_NONE;
}

DX12RenderTexture2D::~DX12RenderTexture2D()
{
	Clear();
}



void DX12RenderTexture2D::SetResource(void * heap)
{
	D3D12_CPU_DESCRIPTOR_HANDLE* Handle = reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(heap);
	Factory->Device->CreateShaderResourceView(TextureBuffer.Get(), &TextureView, *Handle);
}

void DX12RenderTexture2D::AllocUploadBuffer()
{

	bsize SizeWidth = 0;
	bsize SizeDepth = 0;
	{
		SizeWidth = (BearGraphics::BearTextureUtils::GetSizeWidth(TextureDesc.Width, m_format));
		SizeWidth = (SizeWidth + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)&~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT-1);
		SizeDepth = SizeWidth * BearGraphics::BearTextureUtils::GetCountBlock(TextureDesc.Height, m_format);
		SizeDepth = (SizeDepth + D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1)&~(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT-1);
	}
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_UPLOAD);
	auto var2 = CD3DX12_RESOURCE_DESC::Buffer(SizeDepth);
	R_CHK(Factory->Device->CreateCommittedResource(
		&var1,
		D3D12_HEAP_FLAG_NONE,
		&var2,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&UploadBuffer)));

}

void DX12RenderTexture2D::FreeUploadBuffer()
{
	UploadBuffer.Reset();
}
