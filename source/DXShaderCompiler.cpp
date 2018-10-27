#include "DXPCH.h"
#include "directx/D3D11Shader.h"


uint16 GCountVertexShaderCompiler = 0;
uint16 GCountPixelShaderCompiler = 0;

DXVertexShaderCompiler::DXVertexShaderCompiler()
{
	GCountVertexShaderCompiler++;
	shader = 0;
}

bool DXVertexShaderCompiler::compileFromText(const bchar * text, BearCore::BearString & out_error)
{
	if (shader)
		shader->Release();
	ID3D10Blob* errorMessage;
	if (FAILED(D3DX11CompileFromMemory(*BearCore::BearEncoding::ToANSI(text), BearCore::BearString::GetSize(text), 0, 0, 0, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &shader, &errorMessage, 0)))
	{
#ifdef UNICODE
		out_error.append(*BearCore::BearEncoding::ToUTF16((char*)errorMessage->GetBufferPointer(),(char*)errorMessage->GetBufferPointer()+ errorMessage->GetBufferSize()));
#else
		out_error.append((char*)errorMessage->GetBufferPointer(), errorMessage->GetBufferSize());
#endif
		errorMessage->Release();
		return false;
	}
	if (errorMessage)
	{
#ifdef UNICODE
		out_error.append(*BearCore::BearEncoding::ToUTF16((char*)errorMessage->GetBufferPointer(), (char*)errorMessage->GetBufferPointer() + errorMessage->GetBufferSize()));
#else
		out_error.append((char*)errorMessage->GetBufferPointer(), errorMessage->GetBufferSize());
#endif
		errorMessage->Release();

	}
	errorMessage = 0;
	return true;
}

void * DXVertexShaderCompiler::getPointerBuffer()
{
	if(shader)
		return shader->GetBufferPointer();
	return 0;
}

bsize DXVertexShaderCompiler::getSizeBuffer()
{
	if (shader)
		return shader->GetBufferSize();
	return 0;
}

DXVertexShaderCompiler::~DXVertexShaderCompiler()
{
	GCountVertexShaderCompiler--;
	if (shader)
		shader->Release();
}

DXPixelShaderCompiler::DXPixelShaderCompiler()
{
	GCountPixelShaderCompiler++;
	shader = 0;
}

bool DXPixelShaderCompiler::compileFromText(const bchar * text, BearCore::BearString & out_error)
{
	if (shader)
		shader->Release();
	ID3D10Blob* errorMessage;
	if (FAILED(D3DX11CompileFromMemory(*BearCore::BearEncoding::ToANSI(text), BearCore::BearString::GetSize(text), 0, 0, 0, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &shader, &errorMessage, 0)))
	{
#ifdef UNICODE
		out_error.append(*BearCore::BearEncoding::ToUTF16((char*)errorMessage->GetBufferPointer(), (char*)errorMessage->GetBufferPointer() + errorMessage->GetBufferSize()));
#else
		out_error.append((char*)errorMessage->GetBufferPointer(), errorMessage->GetBufferSize());
#endif
		errorMessage->Release();
		return false;
	}
	if (errorMessage)
	{
#ifdef UNICODE
		out_error.append(*BearCore::BearEncoding::ToUTF16((char*)errorMessage->GetBufferPointer(), (char*)errorMessage->GetBufferPointer() + errorMessage->GetBufferSize()));
#else
		out_error.append((char*)errorMessage->GetBufferPointer(), errorMessage->GetBufferSize());
#endif
		errorMessage->Release();
	}
	errorMessage = 0;
	return true;
}

void * DXPixelShaderCompiler::getPointerBuffer()
{
	if (shader)
		return shader->GetBufferPointer();
	return 0;
}

bsize DXPixelShaderCompiler::getSizeBuffer()
{
	if (shader)
		return shader->GetBufferSize();
	return 0;
}

DXPixelShaderCompiler::~DXPixelShaderCompiler()
{
	GCountPixelShaderCompiler--;
	if (shader)
		shader->Release();
}
