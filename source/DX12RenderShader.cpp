#include "DX12PCH.h"


DX12RenderShader::DX12RenderShader(BearGraphics::BearShaderType type):Type(type)
{
}

DX12RenderShader::~DX12RenderShader()
{
}

bool DX12RenderShader::CompileText(const bchar * text, BearCore::BearString & OutError)
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	
	const char*type2text = "";
	switch (Type)
	{
	case BearGraphics::ST_Vertex:
		type2text = "vs_5_0";
		break;
	case BearGraphics::ST_Pixel:
		type2text = "ps_5_0";
		break;
	default:
		break;
	}
	ID3D10Blob* errorMessage;
	bool result = true;
	if (FAILED(D3DCompile(*BearCore::BearEncoding::ToANSI(text), BearCore::BearString::GetSize(text), "", nullptr, nullptr, "main", type2text, compileFlags, 0, &Shader, &errorMessage)))
	{
		result = false;
	}
	if (errorMessage)
	{
#ifdef UNICODE
		OutError.append(*BearCore::BearEncoding::ToUTF16((char*)errorMessage->GetBufferPointer(), (char*)errorMessage->GetBufferPointer() + errorMessage->GetBufferSize()));
#else
		OutError.append((char*)errorMessage->GetBufferPointer(), errorMessage->GetBufferSize());
#endif
		errorMessage->Release();
		return false;
	}
	return result;
}
