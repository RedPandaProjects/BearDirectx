#include "DX12PCH.h"
inline DXGI_FORMAT TranslateVertexFormat(BearGraphics::BearVertexFormat format)
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

DX12RenderPipeline::DX12RenderPipeline(const BearGraphics::BearRenderPipelineDescription & desc)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
	{
		bsize count = 0;
		D3D12_INPUT_ELEMENT_DESC  elemets[16];
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
		auto ps = static_cast<const DX12RenderShader*>(desc.Shaders.Pixel.get());
		if(ps&&ps->IsType(BearGraphics::ST_Pixel))
			Desc.PS = CD3DX12_SHADER_BYTECODE(ps->Shader.Get());

		auto vs = static_cast<const DX12RenderShader*>(desc.Shaders.Vertex.get());
		if (vs&&vs->IsType(BearGraphics::ST_Vertex))
			Desc.VS = CD3DX12_SHADER_BYTECODE(vs->Shader.Get());
	}
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.DepthStencilState.DepthEnable = FALSE;
	Desc.DepthStencilState.StencilEnable = FALSE;
	Desc.SampleMask = UINT_MAX;
	switch (desc.TopologyType)
	{
	case    BearGraphics::TT_POINT_LIST:
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		TopologyType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	case	  BearGraphics::TT_LINE_LIST:
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		TopologyType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case	  BearGraphics::TT_TRIANGLE_LIST:
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		TopologyType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	default:
		break;
	}
	
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	Desc.SampleDesc.Count = 1;

	RootSignature = desc.RootSignature;
	RootSignaturePointer = static_cast<DX12RenderRootSignature*>(RootSignature.get());
	Desc.pRootSignature = RootSignaturePointer->RootSignature.Get();

	R_CHK(Factory->Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&PipelineState)));
}

DX12RenderPipeline::~DX12RenderPipeline()
{

}
