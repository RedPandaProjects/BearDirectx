#include "DXPCH.h"
#include "directx/D3D11Shader.h"


uint16 GCountVertexShaderCompiler = 0;
uint16 GCountPixelShaderCompiler = 0;

DXVertexShaderCompiler::DXVertexShaderCompiler()
{
	GCountVertexShaderCompiler++;
	shader = 0;
}

bool DXVertexShaderCompiler::CompileFromText(const bchar * text, BearCore::BearString & out_error)
{
	if (shader)
		shader->Release();
	ID3D10Blob* errorMessage;
	const char*VS =
#ifdef DX10
		"vs_4_0";
#else
		"vs_5_0";
#endif
	if (FAILED(D3DX11CompileFromMemory(*BearCore::BearEncoding::ToANSI(text), BearCore::BearString::GetSize(text), 0, 0, 0, "main",
		VS
		, D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &shader, &errorMessage, 0)))
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

void * DXVertexShaderCompiler::GetPointerBuffer()
{
	if(shader)
		return shader->GetBufferPointer();
	return 0;
}

bsize DXVertexShaderCompiler::GetSizeBuffer()
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

bool DXPixelShaderCompiler::CompileFromText(const bchar * text, BearCore::BearString & out_error)
{
	if (shader)
		shader->Release();
	ID3D10Blob* errorMessage;
	const char*PS =
#ifdef DX10
		"ps_4_0";
#else
		"ps_5_0";
#endif
	if (FAILED(D3DX11CompileFromMemory(*BearCore::BearEncoding::ToANSI(text), BearCore::BearString::GetSize(text), 0, 0, 0, "main",
		PS

		, D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &shader, &errorMessage, 0)))
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

void * DXPixelShaderCompiler::GetPointerBuffer()
{
	if (shader)
		return shader->GetBufferPointer();
	return 0;
}

bsize DXPixelShaderCompiler::GetSizeBuffer()
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
