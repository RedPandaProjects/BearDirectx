#pragma once
class DX12RenderFactory :public BearRenderBase::BearRenderFactoryBase
{
	BEAR_CLASS_NO_COPY(DX12RenderFactory);
public:
	DX12RenderFactory();
	virtual ~DX12RenderFactory();
	virtual BearRenderBase::BearRenderInterfaceBase* CreateInterface();
	virtual BearRenderBase::BearRenderContextBase* CreateContext();
	virtual BearRenderBase::BearRenderViewportBase* CreateViewport(void * Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearGraphics::BearRenderViewportDescription&Description);
	virtual BearRenderBase::BearRenderShaderBase* CreateShader(BearGraphics::BearShaderType Type);
	inline bool Empty()const { return Device.Get()==0; }
public:
	ComPtr<ID3D12Device> Device;
	ComPtr<IDXGIFactory4> GIFactory;

private:
	void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
};