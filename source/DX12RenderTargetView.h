#pragma once
class DX12RenderTargetView :public BearRenderBase::BearRenderTargetViewBase
{
	BEAR_CLASS_NO_COPY(DX12RenderTargetView);
public:
	DX12RenderTargetView(const BearGraphics::BearRenderTargetViewDescription&Description);
	virtual ~DX12RenderTargetView();
	virtual void Resize(bsize Width, bsize Height);
	virtual void SetResource(void*heap);

	ComPtr<ID3D12DescriptorHeap> Heap;
	DX12RenderTexture2D RTTexture;
	BearGraphics::BearRenderTargetFormat Format;
};
