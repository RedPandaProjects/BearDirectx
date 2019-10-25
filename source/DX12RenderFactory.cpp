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
	if (hardwareAdapter.Get() == 0)return;
	IDXGIOutput *Output;
	hardwareAdapter->EnumOutputs(0, &Output);
	{
		UINT count = 0;

		Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, 0);
		m_GIVideoMode.resize(count);
		Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, &m_GIVideoMode[0]);
	}

	Output->Release();
	if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device))))
	{
		return;
	}

	{
		CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc;
		RootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		R_CHK(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		R_CHK(Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
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
	return  bear_new<DX12RenderShader>(Type);
}

BearRenderBase::BearRenderPipelineBase * DX12RenderFactory::CreatePipeline(const BearGraphics::BearRenderPipelineDescription & Descruotion)
{
	return  bear_new<DX12RenderPipeline>(Descruotion);
}

BearRenderBase::BearRenderIndexBufferBase * DX12RenderFactory::CreateIndexBuffer()
{
	return bear_new<DX12RenderIndexBuffer>();
}

BearRenderBase::BearRenderVertexBufferBase * DX12RenderFactory::CreateVertexBuffer()
{
	return  bear_new<DX12RenderVertexBuffer>();
}

DXGI_MODE_DESC * DX12RenderFactory::FindMode(bsize width, bsize height)
{
	auto begin = m_GIVideoMode.begin();
	auto end = m_GIVideoMode.end();
	while (begin != end)
	{
		if (begin->Width == width && begin->Height == height)
			return &*begin;
		begin++;
	}
	return 0;
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
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
