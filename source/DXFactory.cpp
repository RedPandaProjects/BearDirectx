#include "DXPCH.h"



DXFactory::DXFactory()
{
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&GIFactory))))
		return;
	UINT i = 0;
	bool PerfHUD = false;
	IDXGIAdapter*Adapter;
	while (GIFactory->EnumAdapters(i, &Adapter)!= DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc; 
		Adapter->GetDesc(&desc);
		if (!wcscmp(desc.Description, L"NVIDIA PerfHUD"))
		{
			PerfHUD = true;
			break;
		}
		else 
		{
			Adapter->Release();
			Adapter = 0;
		}
		i++;
	}
	
	IDXGIOutput *Output;
	if (!Adapter)
		GIFactory->EnumAdapters(0, &Adapter);
	Adapter->EnumOutputs(0, &Output);
	if (!Output)
	{
		Adapter->Release();
		GIFactory->Release();
		if (device)device->Release();
		if (deviceContext)deviceContext->Release();
		GIFactory = 0;
		Adapter = 0;
		device = 0;
		deviceContext = 0;
	}


	UINT count= 0;

	// Get the number of display modes available
	Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, 0);

	// Get the list of display modes
	m_GIVideoMode.resize(count);
	Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, &m_GIVideoMode[0]);
	Output->Release();

	D3D_FEATURE_LEVEL Level = D3D_FEATURE_LEVEL_11_0;
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_DRIVER_TYPE DriveType = PerfHUD ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE;
	if (FAILED(D3D11CreateDevice(0, DriveType, NULL, 0, &Level, 1, D3D11_SDK_VERSION, &device,0, &deviceContext)))
	{
		if (Adapter)Adapter->Release();
		GIFactory->Release();
		if(device)device->Release();
		if (deviceContext)deviceContext->Release();
		GIFactory = 0;
		Adapter = 0;
		device = 0;
		deviceContext = 0;
	}
	if(Adapter)
	Adapter->Release();

}

BearRHI::BearRHIInterface * DXFactory::createInterface()
{
	return BearCore::bear_new<DXInterface>();;
}

void DXFactory::destroyInterface(BearRHI::BearRHIInterface * a)
{
	DXInterface*b = (DXInterface*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIRenderTargetView * DXFactory::createRenderTargetView(bsize w, bsize h, BearGraphics::BearRenderTargetFormat format)
{
	return BearCore::bear_new<DXRenderTergetView>(w,h,format);
}

void DXFactory::destroyRenderTargetView(BearRHI::BearRHIRenderTargetView * a)
{
	DXRenderTergetView*b = (DXRenderTergetView*)a;
	BearCore::bear_delete<>(b);
}


BearRHI::BearRHIViewPort * DXFactory::createViewPort(void * win, bsize w, bsize h, bool fullscreen, bool vsync)
{
	return BearCore::bear_new<DXViewPort>(win,w,h,fullscreen,vsync);
}

void DXFactory::destroyViewPort(BearRHI::BearRHIViewPort * a)
{
	DXViewPort*b = (DXViewPort*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHITexture2D * DXFactory::createTexture2D(bsize w, bsize h, bsize mip,  BearGraphics::BearTexturePixelFormat format, bool dynamic, void*data)
{
	return  BearCore::bear_new<DXTexture2D>(w,h,mip,format,dynamic,data);
}

void DXFactory::destroyTexture2D(BearRHI::BearRHITexture2D * a)
{
	DXTexture2D*b = (DXTexture2D*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIVertexBuffer * DXFactory::createVertexBuffer(void * data, bsize size, bool dynamic)
{
	return BearCore::bear_new<DXVertexBuffer>( data,size,dynamic);;
}

void DXFactory::destroyVertexBuffer(BearRHI::BearRHIVertexBuffer * a)
{
	DXVertexBuffer*b = (DXVertexBuffer*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIIndexBuffer * DXFactory::createIndexBuffer(void * data, bsize size, bool dynamic)
{
	return BearCore::bear_new<DXIndexBuffer>( data, size, dynamic);;
}

void DXFactory::destroyIndexBuffer(BearRHI::BearRHIIndexBuffer * a)
{
	DXIndexBuffer*b = (DXIndexBuffer*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIVertexShader * DXFactory::createVertexShader(void * data, bsize size)
{
	auto a = BearCore::bear_new<DXVertexShader>( data, size);;
	if (!a->shader)
	{
		BearCore::bear_delete(a); a = 0;
	}
	return a;
}

void DXFactory::destroyVertexShader(BearRHI::BearRHIVertexShader * a)
{
	DXVertexShader*b = (DXVertexShader*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIPixelShader * DXFactory::createPixelShader(void * data, bsize size)
{

	auto a= BearCore::bear_new<DXPixelShader>( data, size);;
	if (!a->shader) {
		BearCore::bear_delete(a); a = 0;
	}
	return a;
}

void DXFactory::destroyPixelShader(BearRHI::BearRHIPixelShader * a)
{
	DXPixelShader*b = (DXPixelShader*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIVertexShaderCompiler * DXFactory::createVertexShaderCompiler()
{
	return BearCore::bear_new<DXVertexShaderCompiler>();;
}

void DXFactory::destroyVertexShaderCompiler(BearRHI::BearRHIVertexShaderCompiler * a)
{
	DXVertexShaderCompiler*b = (DXVertexShaderCompiler*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIPixelShaderCompiler * DXFactory::createPixelShaderCompiler()
{
	return BearCore::bear_new<DXPixelShaderCompiler>();;
}

void DXFactory::destroyPixelShaderCompiler(BearRHI::BearRHIPixelShaderCompiler * a)
{
	DXPixelShaderCompiler*b = (DXPixelShaderCompiler*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIVertexInputLayout * DXFactory::createVertexInputLayout(const BearGraphics::BearVertexInputLayoutInitializer & initializer, void * data, bsize size)
{
	return BearCore::bear_new<DXVertexInputLayout>( initializer,data,size);;
}

void DXFactory::destroyVertexInputLayout(BearRHI::BearRHIVertexInputLayout * a)
{
	DXVertexInputLayout*b = (DXVertexInputLayout*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIDepthStencilView * DXFactory::createDepthStencilView(bsize w, bsize h, BearGraphics::BearDepthStencilFormat format)
{
	return  BearCore::bear_new<DXDepthStencilView>(w,h,format);
}

void DXFactory::destroyDepthStencilView(BearRHI::BearRHIDepthStencilView * a)
{
	DXDepthStencilView*b = (DXDepthStencilView*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIDepthStencilState * DXFactory::createDepthStencilState(const BearGraphics::BearDepthStencilStateInitializer & initializer)
{
	return  BearCore::bear_new<DXDepthStencilState>( initializer);
}

void DXFactory::destroyDepthStencilState(BearRHI::BearRHIDepthStencilState * a)
{
	DXDepthStencilState*b = (DXDepthStencilState*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHISamplerState * DXFactory::createSamplerState(const BearGraphics::BearSamplerStateInitializer & initializer)
{
	return  BearCore::bear_new<DXSamplerState>( initializer);
}

void DXFactory::destroySamplerState(BearRHI::BearRHISamplerState * a)
{
	DXSamplerState*b = (DXSamplerState*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIBlendState * DXFactory::createBlendState(const BearGraphics::BearBlendStateInitializer & initializer)
{
	return  BearCore::bear_new<DXBlendState>( initializer);
}

void DXFactory::destroyBlendState(BearRHI::BearRHIBlendState * a)
{
	DXBlendState*b = (DXBlendState*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIRasterizerState * DXFactory::createRasterizerState(const BearGraphics::BearRasterizerStateInitializer & initializer)
{
	return  BearCore::bear_new<DXRasterizerState>( initializer);
}

void DXFactory::destroyRasterizerState(BearRHI::BearRHIRasterizerState * a)
{
	DXRasterizerState*b = (DXRasterizerState*)a;
	BearCore::bear_delete<>(b);
}

bool DXFactory::isVoid()
{
	return !device||!deviceContext;
}

DXGI_FORMAT DXFactory::TranslateTextureFromat(BearGraphics::BearTexturePixelFormat format)
{
	switch (format)
	{
	case BearGraphics::TPF_R8:
		return DXGI_FORMAT_R8_UNORM;
		break;
	case BearGraphics::TPF_R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
		break;
	case BearGraphics::TPF_R8G8B8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case BearGraphics::TPF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case BearGraphics::TPF_R32F:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case BearGraphics::TPF_R32G32F:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case BearGraphics::TPF_R32G32B32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case BearGraphics::TPF_R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case BearGraphics::TPF_BC1:
	case BearGraphics::TPF_BC1a:
		return DXGI_FORMAT_BC1_UNORM;
	case BearGraphics::TPF_BC2:
		return DXGI_FORMAT_BC2_UNORM;
	case BearGraphics::TPF_BC3:
		return DXGI_FORMAT_BC3_UNORM;
	case BearGraphics::TPF_BC4:
		return DXGI_FORMAT_BC4_UNORM;
	case BearGraphics::TPF_BC5:
		return DXGI_FORMAT_BC5_UNORM;
	case BearGraphics::TPF_BC6:
		return DXGI_FORMAT_BC6H_UF16;
	case BearGraphics::TPF_BC7:
		return DXGI_FORMAT_BC7_UNORM;
	default:
		BEAR_ASSERT(0);;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT DXFactory::TranslateRenderTargetFromat(BearGraphics::BearRenderTargetFormat format)
{
	switch (format)
	{
	case BearGraphics::RTF_R8:
		return DXGI_FORMAT_R8_UNORM;
		break;
	case BearGraphics::RTF_R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
		break;
	case BearGraphics::RTF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case BearGraphics::RTF_R32F:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case BearGraphics::RTF_R32G32F:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case BearGraphics::RTF_R32G32B32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case BearGraphics::RTF_R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		BEAR_ASSERT(0);;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT DXFactory::TranslateDepthStencillFromat(BearGraphics::BearDepthStencilFormat format)
{
	switch (format)
	{
	case BearGraphics::DSF_DEPTH16:
		return DXGI_FORMAT_D16_UNORM;
		break;
	case BearGraphics::DSF_DEPTH32F:
		return DXGI_FORMAT_D32_FLOAT;
		break;
	case BearGraphics::DSF_DEPTH24_STENCIL8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	case BearGraphics::DSF_DEPTH32F_STENCIL8:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		break;
	default:
		BEAR_ASSERT(0);;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT DXFactory::TranslateInputLayoutElement(BearGraphics::BearInputLayoutElementType format)
{
	switch (format)
	{
	case BearGraphics::ILE_R32G32B32_FLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case BearGraphics::ILE_R32G32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
	case BearGraphics::ILE_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case BearGraphics::ILE_R32G32B32A32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	default:
		BEAR_ASSERT(0);;
		return DXGI_FORMAT_UNKNOWN;
	}

	//return DXGI_FORMAT_UNKNOWN;
}

D3D11_CULL_MODE DXFactory::TranslateRasterizerCullMode(BearGraphics::BearRasterizerCullMode format)
{
	switch (format)
	{
	case BearGraphics::RCM_FRONT:
		return D3D11_CULL_MODE::D3D11_CULL_FRONT;
		break;
	case BearGraphics::RCM_BACK:
		return D3D11_CULL_MODE::D3D11_CULL_BACK;
		break;
	case BearGraphics::RCM_NONE:
		return D3D11_CULL_MODE::D3D11_CULL_NONE;
		break;
	default:
		BEAR_ASSERT(0);;
	}
	return D3D11_CULL_MODE::D3D11_CULL_NONE;
}

D3D11_FILL_MODE DXFactory::TranslateRasterizerFillMode(BearGraphics::BearRasterizerFillMode format)
{
	switch (format)
	{
	case BearGraphics::RFM_WIREFRAME:
		return D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
		break;
	case BearGraphics::RFM_SOLID:
		return D3D11_FILL_MODE::D3D11_FILL_SOLID;
		break;
	default:
		BEAR_ASSERT(0);;
	}
	return D3D11_FILL_MODE::D3D11_FILL_SOLID;
}

D3D11_BLEND DXFactory::TranslateBlendFactor(BearGraphics::BearBlendFactor format)
{
	switch (format)
	{
	case BearGraphics::BF_ZERO:
		return D3D11_BLEND::D3D11_BLEND_ZERO;
		break;
	case BearGraphics::BF_ONE:
		return D3D11_BLEND::D3D11_BLEND_ONE;
		break;
	case BearGraphics::BF_SRC_COLOR:
		return D3D11_BLEND::D3D11_BLEND_SRC_COLOR;
		break;
	case BearGraphics::BF_INV_SRC_COLOR:
		return D3D11_BLEND::D3D11_BLEND_INV_SRC_COLOR;
		break;
	case BearGraphics::BF_SRC_ALPHA:
		return D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		break;
	case BearGraphics::BF_INV_SRC_ALPHA:
		return D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		break;
	case BearGraphics::BF_DEST_ALPHA:
		return D3D11_BLEND::D3D11_BLEND_DEST_ALPHA;
		break;
	case BearGraphics::BF_INV_DEST_ALPHA:
		return D3D11_BLEND::D3D11_BLEND_INV_DEST_ALPHA;
		break;
	case BearGraphics::BF_DEST_COLOR:
		return D3D11_BLEND::D3D11_BLEND_DEST_COLOR;
		break;
	case BearGraphics::BF_INV_DEST_COLOR:
		return D3D11_BLEND::D3D11_BLEND_INV_DEST_COLOR;
		break;
	case BearGraphics::BF_BLEND_FACTOR:
		return D3D11_BLEND::D3D11_BLEND_BLEND_FACTOR;
		break;
	case BearGraphics::BF_INV_BLEND_FACTOR:
		return D3D11_BLEND::D3D11_BLEND_INV_BLEND_FACTOR;
		break;
	default:
		BEAR_ASSERT(0);;
	}
	return D3D11_BLEND_ZERO;
}

D3D11_BLEND_OP DXFactory::TranslateBlendOp(BearGraphics::BearBlendOp format)
{
	switch (format)
	{
	case BearGraphics::BO_ADD:
		return D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		break;
	case BearGraphics::BO_SUBTRACT:
		return D3D11_BLEND_OP::D3D11_BLEND_OP_SUBTRACT;
		break;
	case BearGraphics::BO_REV_SUBTRACT:
		return D3D11_BLEND_OP::D3D11_BLEND_OP_REV_SUBTRACT;
		break;
	case BearGraphics::BO_MIN:
		return D3D11_BLEND_OP::D3D11_BLEND_OP_MIN;
		break;
	case BearGraphics::BO_MAX:
		return D3D11_BLEND_OP::D3D11_BLEND_OP_MAX;
		break;
	default:
		BEAR_ASSERT(0);;
	}
	return D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
}

D3D11_COMPARISON_FUNC DXFactory::TranslateCompareFunction(BearGraphics::BearCompareFunction format)
{
	switch (format)
	{
	case BearGraphics::CF_NEVER:
		return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
		break;
	case BearGraphics::CF_ALWAYS:
		return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
		break;
	case BearGraphics::CF_EQUAL:
		return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_EQUAL;
		break;
	case BearGraphics::CF_NOTEQUAL:
		return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NOT_EQUAL;
		break;
	case BearGraphics::CF_LESS:
		return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
		break;
	case BearGraphics::CF_GREATER:
		return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER;
		break;
	case BearGraphics::CF_LESSEQUAL:
		return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
		break;
	case BearGraphics::CF_GREATEREQUAL:
		return D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER_EQUAL;
		break;
	default:
		BEAR_ASSERT(0);;
	}
	return  D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
}

D3D11_STENCIL_OP DXFactory::TranslateStencilOp(BearGraphics::BearStencilOp format)
{
	switch (format)
	{
	case BearGraphics::SO_KEEP:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		break;
	case BearGraphics::SO_ZERO:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
		break;
	case BearGraphics::SO_REPLACE:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
		break;
	case BearGraphics::SO_INCR_SAT:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_INCR_SAT;
		break;
	case BearGraphics::SO_DECR_SAT:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_DECR_SAT;
		break;
	case BearGraphics::SO_INVERT:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_INVERT;
		break;
	case BearGraphics::SO_INCR:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_INCR;
		break;
	case BearGraphics::SO_DECR:
		return D3D11_STENCIL_OP::D3D11_STENCIL_OP_DECR;
		break;
	default:
		BEAR_ASSERT(0);;
	}
	return  D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
}

D3D11_TEXTURE_ADDRESS_MODE DXFactory::TranslateSamplerAddressMode(BearGraphics::BearSamplerAddressMode format)
{
	switch (format)
	{
	case BearGraphics::SAM_WRAP:
		return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	case BearGraphics::SAM_MIRROR:
		D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	case BearGraphics::SAM_CLAMP:
		D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case BearGraphics::SAM_BORDER:
		D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
		break;
	default:
		BEAR_ASSERT(0);;;
	}
	return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
}



DXFactory::~DXFactory()
{
	
	m_GIVideoMode.clear();
	if(GIFactory)GIFactory->Release();
	if (device)device->Release();
	if (deviceContext)deviceContext->Release();
}

DXGI_MODE_DESC * DXFactory::findMode(bsize width, bsize height)
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
