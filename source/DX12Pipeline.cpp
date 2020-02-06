#include "DX12PCH.h"
bsize PipelineCounter = 0;
inline DXGI_FORMAT TranslateVertexFormat(BearVertexFormat format)
{
	switch (format)
	{
	case VF_R16G16_SINT:
		return DXGI_FORMAT_R16G16_SINT;
	case VF_R16G16B16A16_SINT:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case VF_R16G16_FLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;
	case VF_R16G16B16A16_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case VF_R32G32B32A32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case VF_R32G32B32_FLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case VF_R32G32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
		
	case VF_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

	case VF_R32_INT:
		return DXGI_FORMAT_R32_SINT;
	case VF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case VF_R8G8:
		return DXGI_FORMAT_R8G8_UINT;
	case VF_R8:
		return DXGI_FORMAT_R8_UINT;
	default:
		BEAR_ASSERT(0);;
		return DXGI_FORMAT_UNKNOWN;
	}

	//return DXGI_FORMAT_UNKNOWN;
}

DX12Pipeline::DX12Pipeline(const BearPipelineDescription & desc)
{
	PipelineCounter++;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
	D3D12_INPUT_ELEMENT_DESC  elemets[16];
	{
		bsize count = 0;
		
		for (bsize i = 0; i < 16 && !desc.InputLayout.Elements[i].empty(); i++)
		{
			auto& cElement = desc.InputLayout.Elements[i];
			elemets[i].Format = TranslateVertexFormat(cElement.Type);
			elemets[i].SemanticName = *desc.InputLayout.Elements[i].Name;
			elemets[i].InputSlot = 0;

			elemets[i].AlignedByteOffset = static_cast<UINT>(desc.InputLayout.Elements[i].Offset);
			elemets[i].InputSlotClass = desc.InputLayout.Elements[i].IsInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			elemets[i].InstanceDataStepRate = desc.InputLayout.Elements[i].IsInstance ? 1 : 0;
			elemets[i].SemanticIndex = static_cast<UINT>(desc.InputLayout.Elements[i].SemanticIndex);
			count++;
		}
		Desc.InputLayout.pInputElementDescs = elemets;
		Desc.InputLayout.NumElements =static_cast<UINT>( count);
	};
	{
		auto ps = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(desc.Shaders.Pixel.get()));
		if(ps&&ps->IsType(ST_Pixel))
			Desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetPointer(), ps->GetSize());

		auto vs = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(desc.Shaders.Vertex.get()));
		if (vs&&vs->IsType(ST_Vertex))
			Desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetPointer(), vs->GetSize());
	}
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	{
		bear_fill(Desc.RasterizerState);
		Desc.RasterizerState.AntialiasedLineEnable = false;
		Desc.RasterizerState.CullMode = DX12Factory::Translate(desc.RasterizerState.CullMode);
		Desc.RasterizerState.DepthBias = (int32)(desc.RasterizerState.DepthBias * (float)(1 << 24));
		Desc.RasterizerState.DepthBiasClamp = 0.0f;
		Desc.RasterizerState.DepthClipEnable = true;
		Desc.RasterizerState.FillMode = DX12Factory::Translate(desc.RasterizerState.FillMode);;
		Desc.RasterizerState.FrontCounterClockwise = false;
		Desc.RasterizerState.MultisampleEnable = false;
		Desc.RasterizerState.FrontCounterClockwise = desc.RasterizerState.FrontFace == RFF_COUNTER_CLOCKWISE;
		Desc.RasterizerState.SlopeScaledDepthBias = desc.RasterizerState.SlopeScaleDepthBias;
	}
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	{
		Desc.BlendState.IndependentBlendEnable = true;
		Desc.BlendState.AlphaToCoverageEnable = desc.BlendState.AlphaToCoverageEnable;
		for (bsize i = 0; i < 8; i++)
		{
			Desc.BlendState.RenderTarget[i].BlendEnable = desc.BlendState.RenderTarget[i].Enable;
			Desc.BlendState.RenderTarget[i].BlendOp = DX12Factory::Translate(desc.BlendState.RenderTarget[i].Color);
			Desc.BlendState.RenderTarget[i].BlendOpAlpha = DX12Factory::Translate(desc.BlendState.RenderTarget[i].Alpha);
			Desc.BlendState.RenderTarget[i].SrcBlend = DX12Factory::Translate(desc.BlendState.RenderTarget[i].ColorSrc);
			Desc.BlendState.RenderTarget[i].DestBlend = DX12Factory::Translate(desc.BlendState.RenderTarget[i].ColorDst);
			Desc.BlendState.RenderTarget[i].SrcBlendAlpha = DX12Factory::Translate(desc.BlendState.RenderTarget[i].AlphaSrc);
			Desc.BlendState.RenderTarget[i].DestBlendAlpha = DX12Factory::Translate(desc.BlendState.RenderTarget[i].AlphaDst);
			if (desc.BlendState.RenderTarget[i].ColorWriteMask & CWM_R)
				Desc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_RED;
			if (desc.BlendState.RenderTarget[i].ColorWriteMask & CWM_G)
				Desc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
			if (desc.BlendState.RenderTarget[i].ColorWriteMask & CWM_B)
				Desc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
			if (desc.BlendState.RenderTarget[i].ColorWriteMask & CWM_A)
				Desc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
		}

	}
	{
		bear_fill(Desc.DepthStencilState);
		Desc.DepthStencilState.DepthEnable = desc.DepthStencilState.DepthEnable;
		Desc.DepthStencilState.DepthFunc = DX12Factory::Translate(desc.DepthStencilState.DepthTest);
		Desc.DepthStencilState.DepthWriteMask = desc.DepthStencilState.EnableDepthWrite ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
		Desc.DepthStencilState.StencilEnable = desc.DepthStencilState.StencillEnable;
		Desc.DepthStencilState.StencilReadMask = desc.DepthStencilState.StencilReadMask;
		Desc.DepthStencilState.StencilWriteMask = desc.DepthStencilState.StencilWriteMask;
		Desc.DepthStencilState.FrontFace.StencilDepthFailOp = DX12Factory::Translate(desc.DepthStencilState.FrontFace.StencilDepthFailOp);
		Desc.DepthStencilState.FrontFace.StencilFailOp = DX12Factory::Translate(desc.DepthStencilState.FrontFace.StencilFailOp);
		Desc.DepthStencilState.FrontFace.StencilPassOp = DX12Factory::Translate(desc.DepthStencilState.FrontFace.StencilPassOp);
		Desc.DepthStencilState.FrontFace.StencilFunc = DX12Factory::Translate(desc.DepthStencilState.FrontFace.StencilTest);
		if (desc.DepthStencilState.BackStencillEnable)
		{
			Desc.DepthStencilState.BackFace.StencilDepthFailOp = DX12Factory::Translate(desc.DepthStencilState.BackFace.StencilDepthFailOp);
			Desc.DepthStencilState.BackFace.StencilFailOp = DX12Factory::Translate(desc.DepthStencilState.BackFace.StencilFailOp);
			Desc.DepthStencilState.BackFace.StencilPassOp = DX12Factory::Translate(desc.DepthStencilState.BackFace.StencilPassOp);
			Desc.DepthStencilState.BackFace.StencilFunc = DX12Factory::Translate(desc.DepthStencilState.BackFace.StencilTest);
		}
		else
		{
			Desc.DepthStencilState.BackFace = Desc.DepthStencilState.FrontFace;
		}

	}
	Desc.SampleMask = UINT_MAX;
	switch (desc.TopologyType)
	{
	case    TT_POINT_LIST:
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		TopologyType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	case	  TT_LINE_LIST:
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		TopologyType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case	  TT_LINE_STRIP:
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		TopologyType = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	case	  TT_TRIANGLE_LIST:
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		TopologyType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	case	  TT_TRIANGLE_STRIP:
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		TopologyType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		break;
	default:
		break;
	}
	

	if (desc.RenderPass.empty())
	{
		Desc.NumRenderTargets = 1;
		Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	}
	else
	{
		const DX12RenderPass* RenderPass = static_cast<const DX12RenderPass*>(desc.RenderPass.get());
		Desc.NumRenderTargets =  static_cast<UINT>(RenderPass->CountRenderTarget);
		for (bsize i = 0; i < Desc.NumRenderTargets; i++)
		{
			Desc.RTVFormats[i] = DX12Factory::Translation(RenderPass->Description.RenderTargets[i].Format);

			
		}
		if (RenderPass->Description.DepthStencil.Format != DSF_NONE)
		{
			Desc.DSVFormat = DX12Factory::Translation(RenderPass->Description.DepthStencil.Format);
		}
	}

	
	Desc.SampleDesc.Count = 1;


	RootSignature = desc.RootSignature;
	BEAR_ASSERT(RootSignature.empty() == false);
	RootSignaturePointer = static_cast<DX12RootSignature*>(RootSignature.get());
	Desc.pRootSignature = RootSignaturePointer->RootSignature.Get();


	R_CHK(Factory->Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&PipelineState)));
}

DX12Pipeline::~DX12Pipeline()
{
	PipelineCounter--;
}

void DX12Pipeline::Set(
#ifdef RTX
	ID3D12GraphicsCommandList4
#else 
	ID3D12GraphicsCommandList
#endif
	
	* CommandList)
{
	CommandList->SetPipelineState(PipelineState.Get());
	CommandList->IASetPrimitiveTopology(TopologyType);
	RootSignaturePointer->Set(CommandList);
}



