#include "DX12PCH.h"
DX12RenderFactory::DX12RenderFactory()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&GIFactory))))
	{
		return;
	}
	ComPtr<IDXGIAdapter1> hardwareAdapter;

	GetHardwareAdapter(GIFactory.Get(), &hardwareAdapter);

	if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device))))
	{
		return;
	}


}
DX12RenderFactory::~DX12RenderFactory()
{
}
BearRenderBase::BearRenderInterfaceBase* DX12RenderFactory::CreateInterface()
{
	return bear_new<DX12RenderInterface>();
}
BearRenderBase::BearRenderContextBase * DX12RenderFactory::CreateContext()
{
	return bear_new<DX12RenderContext>();;
}
BearRenderBase::BearRenderViewportBase * DX12RenderFactory::CreateViewport(void * Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearGraphics::BearRenderViewportDescription&Description)
{
	return  bear_new<DX12RenderViewport>(Handle, Width, Height, Fullscreen, VSync, Description);;;
}

BearRenderBase::BearRenderShaderBase * DX12RenderFactory::CreateShader(BearGraphics::BearShaderType Type)
{
	return nullptr;
}

void DX12RenderFactory::GetHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
