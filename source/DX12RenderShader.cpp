#include "DX12PCH.h"
inline bool IsRtxShader(BearGraphics::BearShaderType Type)
{
	switch (Type)
	{
	case BearGraphics::ST_RayGeneration:
	case BearGraphics::ST_Hit:
	case BearGraphics::ST_Miss:
		return true;
	default:
		return false;
	}
}

DX12RenderShader::DX12RenderShader(BearGraphics::BearShaderType type):Type(type)
{
	RTXShader = 0;
}

DX12RenderShader::~DX12RenderShader()
{
	if (RTXShader)RTXShader->Release();
}

bool DX12RenderShader::CompileText(const bchar * text, BearCore::BearString & OutError)
{
	if (IsRtxShader(Type))
	{
		IDxcBlobEncoding* PointerTextBlob;
		auto buffer = BearEncoding::ToANSI(text);
		R_CHK(Factory->DxcLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)*buffer, (uint32_t)BearString::GetSize(text), 0, &PointerTextBlob));

		IDxcOperationResult* Result;
		R_CHK(Factory->DxcCompiler->Compile(PointerTextBlob, L"none", L"", L"lib_6_3", nullptr, 0, nullptr, 0, Factory->DxcIncludeHandler, &Result));
		
		HRESULT resultCode;
		R_CHK(Result->GetStatus(&resultCode));
		if (FAILED(resultCode))
		{
			IDxcBlobEncoding* pError;
			R_CHK(Result->GetErrorBuffer(&pError));
			// Convert error blob to a string
			std::vector<char> infoLog(pError->GetBufferSize() + 1);
			memcpy(infoLog.data(), pError->GetBufferPointer(), pError->GetBufferSize());
			infoLog[pError->GetBufferSize()] = 0;

			std::string errorMsg = "Shader Compiler Error:\n";
			errorMsg.append(infoLog.data());
			pError->Release();
			OutError.assign(*BearEncoding::ToCurrent(errorMsg.c_str()));
			return false;
		}

	
		R_CHK(Result->GetResult(&RTXShader));
		Result->Release();
		PointerTextBlob->Release();
		return true;
	}
	else
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
}
