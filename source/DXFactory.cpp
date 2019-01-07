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
		if (wcscmp(desc.Description, L"NVIDIA PerfHUD")==0)
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
	if (FAILED(D3D11CreateDevice(0, DriveType, NULL, flags, &Level, 1, D3D11_SDK_VERSION, &device,0, &deviceContext)))
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
	m_PerfHUD = PerfHUD;
}

BearRHI::BearRHIInterface * DXFactory::CreateInterface()
{
	return BearCore::bear_new<DXInterface>();;
}

void DXFactory::DestroyInterface(BearRHI::BearRHIInterface * a)
{
	DXInterface*b = (DXInterface*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIDefaultManager * DXFactory::CreateDefaultManager()
{
	return BearCore::bear_new<DXDefaultManager>();;
}

void DXFactory::DestroyDefaultManager(BearRHI::BearRHIDefaultManager * a)
{
	DXDefaultManager*b = (DXDefaultManager*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIRenderTargetView * DXFactory::CreateRenderTargetView(bsize w, bsize h, BearGraphics::BearRenderTargetFormat format)
{
	return BearCore::bear_new<DXRenderTergetView>(w,h,format);
}

void DXFactory::DestroyRenderTargetView(BearRHI::BearRHIRenderTargetView * a)
{
	DXRenderTergetView*b = (DXRenderTergetView*)a;
	BearCore::bear_delete<>(b);
}


BearRHI::BearRHIViewport * DXFactory::CreateViewport(void * win, bsize w, bsize h, bool fullscreen, bool vsync)
{
	return BearCore::bear_new<DXViewport>(win,w,h,fullscreen,vsync);
}

void DXFactory::DestroyViewport(BearRHI::BearRHIViewport * a)
{
	DXViewport*b = (DXViewport*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHITexture2D * DXFactory::CreateTexture2D(bsize w, bsize h, bsize mip,  BearGraphics::BearTexturePixelFormat format, bool dynamic, void*data)
{
	return  BearCore::bear_new<DXTexture2D>(w,h,mip,format,dynamic,data);
}

void DXFactory::DestroyTexture2D(BearRHI::BearRHITexture2D * a)
{
	DXTexture2D*b = (DXTexture2D*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHITexture2DArray * DXFactory::CreateTexture2DArray(bsize w, bsize h, bsize depth, bsize mip, BearGraphics::BearTexturePixelFormat format, void * data)
{
	return  BearCore::bear_new<DXTexture2DArray>(w, h, depth,mip, format, data);
}

void DXFactory::DestroyTexture2DArray(BearRHI::BearRHITexture2DArray * a)
{
	DXTexture2DArray*b = (DXTexture2DArray*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIVertexBuffer * DXFactory::CreateVertexBuffer(void * data, bsize size, bool dynamic)
{
	return BearCore::bear_new<DXVertexBuffer>( data,size,dynamic);;
}

void DXFactory::DestroyVertexBuffer(BearRHI::BearRHIVertexBuffer * a)
{
	DXVertexBuffer*b = (DXVertexBuffer*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIIndexBuffer * DXFactory::CreateIndexBuffer(void * data, bsize size, bool dynamic)
{
	return BearCore::bear_new<DXIndexBuffer>( data, size, dynamic);;
}

void DXFactory::DestroyIndexBuffer(BearRHI::BearRHIIndexBuffer * a)
{
	DXIndexBuffer*b = (DXIndexBuffer*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIVertexShader * DXFactory::CreateVertexShader(void * data, bsize size)
{
	auto a = BearCore::bear_new<DXVertexShader>( data, size);;
	if (!a->shader)
	{
		BearCore::bear_delete(a); a = 0;
	}
	return a;
}

void DXFactory::DestroyVertexShader(BearRHI::BearRHIVertexShader * a)
{
	DXVertexShader*b = (DXVertexShader*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIPixelShader * DXFactory::CreatePixelShader(void * data, bsize size)
{

	auto a= BearCore::bear_new<DXPixelShader>( data, size);;
	if (!a->shader) {
		BearCore::bear_delete(a); a = 0;
	}
	return a;
}

void DXFactory::DestroyPixelShader(BearRHI::BearRHIPixelShader * a)
{
	DXPixelShader*b = (DXPixelShader*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIVertexShaderCompiler * DXFactory::CreateVertexShaderCompiler()
{
	return BearCore::bear_new<DXVertexShaderCompiler>();;
}

void DXFactory::DestroyVertexShaderCompiler(BearRHI::BearRHIVertexShaderCompiler * a)
{
	DXVertexShaderCompiler*b = (DXVertexShaderCompiler*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIPixelShaderCompiler * DXFactory::CreatePixelShaderCompiler()
{
	return BearCore::bear_new<DXPixelShaderCompiler>();;
}

void DXFactory::DestroyPixelShaderCompiler(BearRHI::BearRHIPixelShaderCompiler * a)
{
	DXPixelShaderCompiler*b = (DXPixelShaderCompiler*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIVertexState * DXFactory::CreateVertexState(const BearGraphics::BearVertexStateInitializer & initializer, void * data, bsize size)
{
	return BearCore::bear_new<DXVertexState>(initializer,data,size);;
}

void DXFactory::DestroyVertexState(BearRHI::BearRHIVertexState * a)
{
	DXVertexState*b = (DXVertexState*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIDepthStencilView * DXFactory::CreateDepthStencilView(bsize w, bsize h, BearGraphics::BearDepthStencilFormat format)
{
	return  BearCore::bear_new<DXDepthStencilView>(w,h,format);
}

void DXFactory::DestroyDepthStencilView(BearRHI::BearRHIDepthStencilView * a)
{
	DXDepthStencilView*b = (DXDepthStencilView*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIDepthStencilState * DXFactory::CreateDepthStencilState(const BearGraphics::BearDepthStencilStateInitializer & initializer)
{
	return  BearCore::bear_new<DXDepthStencilState>( initializer);
}

void DXFactory::DestroyDepthStencilState(BearRHI::BearRHIDepthStencilState * a)
{
	DXDepthStencilState*b = (DXDepthStencilState*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHISamplerState * DXFactory::CreateSamplerState(const BearGraphics::BearSamplerStateInitializer & initializer)
{
	return  BearCore::bear_new<DXSamplerState>( initializer);
}

void DXFactory::DestroySamplerState(BearRHI::BearRHISamplerState * a)
{
	DXSamplerState*b = (DXSamplerState*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIBlendState * DXFactory::CreateBlendState(const BearGraphics::BearBlendStateInitializer & initializer)
{
	return  BearCore::bear_new<DXBlendState>( initializer);
}

void DXFactory::DestroyBlendState(BearRHI::BearRHIBlendState * a)
{
	DXBlendState*b = (DXBlendState*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIRasterizerState * DXFactory::CreateRasterizerState(const BearGraphics::BearRasterizerStateInitializer & initializer)
{
	return  BearCore::bear_new<DXRasterizerState>( initializer);
}

void DXFactory::DestroyRasterizerState(BearRHI::BearRHIRasterizerState * a)
{
	DXRasterizerState*b = (DXRasterizerState*)a;
	BearCore::bear_delete<>(b);
}

BearRHI::BearRHIShaderConstants * DXFactory::CreateShaderConstants(const BearGraphics::BearShaderConstantsInitializer & initializer, bool dynamic)
{
	return BearCore::bear_new<DXShaderConstants>(initializer, dynamic);;
}

void DXFactory::DestroyShaderConstants(BearRHI::BearRHIShaderConstants * a)
{
	BearCore::bear_delete<DXShaderConstants>(reinterpret_cast<DXShaderConstants*>(a));
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

DXGI_FORMAT DXFactory::TranslateVertexFormat(BearGraphics::BearVertexFormat format)
{
	switch (format)
	{
	case BearGraphics::VF_R32G32B32A32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case BearGraphics::VF_R32G32B32_FLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case BearGraphics::VF_R32G32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
	case BearGraphics::VF_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

	case BearGraphics::VF_R32_INT:
		return DXGI_FORMAT_R32_SINT;
	case BearGraphics::VF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case BearGraphics::VF_R8G8:
		return DXGI_FORMAT_R8G8_UINT;
	case BearGraphics::VF_R8:
		return DXGI_FORMAT_R8_UINT;
	default:
		BEAR_ASSERT(0);;
		return DXGI_FORMAT_UNKNOWN;
	}

	//return DXGI_FORMAT_UNKNOWN;
}

bsize DXFactory::GetSizeVertexFormat(BearGraphics::BearVertexFormat format)
{
	switch (format)
	{
	case BearGraphics::VF_R32G32B32A32_FLOAT:
		return 4 * 4;
	case BearGraphics::VF_R32G32B32_FLOAT:
		return 4 * 3;
	case BearGraphics::VF_R32G32_FLOAT:
		return 4 * 2;
	case BearGraphics::VF_R32_FLOAT:
		return 4 * 1;
	case BearGraphics::VF_R32_INT:
		return 4 ;
	case BearGraphics::VF_R8G8B8A8:
		return 4;
	case BearGraphics::VF_R8G8:
		return 2;
	case BearGraphics::VF_R8:
		return 1;
	default:
		BEAR_ASSERT(0);;
		return 0;
	}
}

bsize DXFactory::GetSizeConstantFormat(BearGraphics::BearConstantFormat format)
{
	switch (format)
	{
	case BearGraphics::CF_R32G32B32A32_FLOAT:
		return 4 * 4;
	case BearGraphics::CF_R32G32B32_FLOAT:
		return 4 * 3;
	case BearGraphics::CF_R32G32_FLOAT:
		return 4 * 2;
	case BearGraphics::CF_R32_FLOAT:
		return 4 * 1;
	case BearGraphics::CF_R32_INT:
		return 4;
	case BearGraphics::CF_MATRIX:
		return 4*4*4;
	default:
		BEAR_ASSERT(0);;
		return 0;
	}
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
