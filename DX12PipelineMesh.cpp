#include "DX12PCH.h"
bsize PipelineMeshCounter = 0;
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
		BEAR_CHECK(0);;
		return DXGI_FORMAT_UNKNOWN;
	}

	//return DXGI_FORMAT_UNKNOWN;
}
#ifdef DX12UTIMATE
namespace
{
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS> CD3DX12_PIPELINE_STATE_STREAM_AS;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS> CD3DX12_PIPELINE_STATE_STREAM_MS;

	struct MeshShaderPsoDesc
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_AS                    AS;
		CD3DX12_PIPELINE_STATE_STREAM_MS                    MS;
		CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencilState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DepthFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTFormats;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK           SampleMask;
	};
}
#endif
DX12PipelineMesh::DX12PipelineMesh(const BearPipelineMeshDescription & desc)
{
	PipelineMeshCounter++;
#ifdef DX12UTIMATE
	MeshShaderPsoDesc Desc = {};

	{
		auto ms = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(desc.Shaders.Mesh.get()));
		if (ms && ms->IsType(ST_Mesh))
			Desc.MS = CD3DX12_SHADER_BYTECODE(ms->GetPointer(), ms->GetSize());

		auto as = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(desc.Shaders.Amplification.get()));
		if (as && as->IsType(ST_Hull))
			Desc.AS = CD3DX12_SHADER_BYTECODE(as->GetPointer(), as->GetSize());

		auto ps = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(desc.Shaders.Pixel.get()));
		if(ps&&ps->IsType(ST_Pixel))
			Desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetPointer(), ps->GetSize());

	}
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	{
		CD3DX12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
		RasterizerState.AntialiasedLineEnable = false;
		RasterizerState.FrontCounterClockwise = false;
		RasterizerState.MultisampleEnable = false;

		RasterizerState.CullMode = DX12Factory::Translate(desc.RasterizerState.CullMode);
		RasterizerState.FillMode = DX12Factory::Translate(desc.RasterizerState.FillMode);;
		RasterizerState.FrontCounterClockwise = desc.RasterizerState.FrontFace == RFF_COUNTER_CLOCKWISE;


		RasterizerState.SlopeScaledDepthBias = desc.RasterizerState.DepthBiasEnable ? desc.RasterizerState.DepthSlopeScaleBias:0;
		RasterizerState.DepthBias = desc.RasterizerState.DepthBiasEnable ? (int)(desc.RasterizerState.DepthBias * (float)(1 << 24)):0;

		RasterizerState.DepthBiasClamp = desc.RasterizerState.DepthClampEnable ? desc.RasterizerState.DepthClmapBias : 0.f;
		Desc.RasterizerState = RasterizerState;
	}
	{
		CD3DX12_BLEND_DESC BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
		BlendState.IndependentBlendEnable = desc.BlendState.IndependentBlendEnable;
		BlendState.AlphaToCoverageEnable = desc.MultisampleState.AlphaToCoverageEnable;
		for (bsize i = 0; i < 8; i++)
		{
			BlendState.RenderTarget[i].BlendEnable = desc.BlendState.RenderTarget[i].Enable;
			BlendState.RenderTarget[i].BlendOp = DX12Factory::Translate(desc.BlendState.RenderTarget[i].Color);
			BlendState.RenderTarget[i].BlendOpAlpha = DX12Factory::Translate(desc.BlendState.RenderTarget[i].Alpha);
			BlendState.RenderTarget[i].SrcBlend = DX12Factory::Translate(desc.BlendState.RenderTarget[i].ColorSrc);
			BlendState.RenderTarget[i].DestBlend = DX12Factory::Translate(desc.BlendState.RenderTarget[i].ColorDst);
			BlendState.RenderTarget[i].SrcBlendAlpha = DX12Factory::Translate(desc.BlendState.RenderTarget[i].AlphaSrc);
			BlendState.RenderTarget[i].DestBlendAlpha = DX12Factory::Translate(desc.BlendState.RenderTarget[i].AlphaDst);
			if (desc.BlendState.RenderTarget[i].ColorWriteMask & CWM_R)
				BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_RED;
			if (desc.BlendState.RenderTarget[i].ColorWriteMask & CWM_G)
				BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
			if (desc.BlendState.RenderTarget[i].ColorWriteMask & CWM_B)
				BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
			if (desc.BlendState.RenderTarget[i].ColorWriteMask & CWM_A)
				BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
		}
		Desc.BlendState = BlendState;

	}
	{
		CD3DX12_DEPTH_STENCIL_DESC DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());
		DepthStencilState.DepthEnable = desc.DepthStencilState.DepthEnable;
		DepthStencilState.DepthFunc = DX12Factory::Translate(desc.DepthStencilState.DepthTest);
		DepthStencilState.DepthWriteMask = desc.DepthStencilState.EnableDepthWrite ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
		DepthStencilState.StencilEnable = desc.DepthStencilState.StencillEnable;
		DepthStencilState.StencilReadMask = desc.DepthStencilState.StencilReadMask;
		DepthStencilState.StencilWriteMask = desc.DepthStencilState.StencilWriteMask;
		DepthStencilState.FrontFace.StencilDepthFailOp = DX12Factory::Translate(desc.DepthStencilState.FrontFace.StencilDepthFailOp);
		DepthStencilState.FrontFace.StencilFailOp = DX12Factory::Translate(desc.DepthStencilState.FrontFace.StencilFailOp);
		DepthStencilState.FrontFace.StencilPassOp = DX12Factory::Translate(desc.DepthStencilState.FrontFace.StencilPassOp);
		DepthStencilState.FrontFace.StencilFunc = DX12Factory::Translate(desc.DepthStencilState.FrontFace.StencilTest);
		if (desc.DepthStencilState.BackStencillEnable)
		{
			DepthStencilState.BackFace.StencilDepthFailOp = DX12Factory::Translate(desc.DepthStencilState.BackFace.StencilDepthFailOp);
			DepthStencilState.BackFace.StencilFailOp = DX12Factory::Translate(desc.DepthStencilState.BackFace.StencilFailOp);
			DepthStencilState.BackFace.StencilPassOp = DX12Factory::Translate(desc.DepthStencilState.BackFace.StencilPassOp);
			DepthStencilState.BackFace.StencilFunc = DX12Factory::Translate(desc.DepthStencilState.BackFace.StencilTest);
		}
		else
		{
			DepthStencilState.BackFace = DepthStencilState.FrontFace;
		}
		Desc.DepthStencilState = DepthStencilState;
	}
	Desc.SampleMask = UINT_MAX;
	
	Desc.DepthFormat = DXGI_FORMAT_UNKNOWN;
	if (desc.RenderPass.empty())
	{
		D3D12_RT_FORMAT_ARRAY DescRT;
		DescRT.NumRenderTargets = 1;
		DescRT.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.RTFormats = DescRT;
	}
	else
	{
		D3D12_RT_FORMAT_ARRAY DescRT;
		const DX12RenderPass* RenderPass = static_cast<const DX12RenderPass*>(desc.RenderPass.get());
		DescRT.NumRenderTargets =  static_cast<UINT>(RenderPass->CountRenderTarget);
		for (bsize i = 0; i < DescRT.NumRenderTargets; i++)
		{
			DescRT.RTFormats[i] = DX12Factory::Translation(RenderPass->Description.RenderTargets[i].Format);

			
		}
		for (bsize i = DescRT.NumRenderTargets; i < 8; i++)
		{
			DescRT.RTFormats[i] = DXGI_FORMAT_UNKNOWN;
		}
		if (RenderPass->Description.DepthStencil.Format != DSF_NONE)
		{
			Desc.DepthFormat = DX12Factory::Translation(RenderPass->Description.DepthStencil.Format);
		}
		Desc.RTFormats = DescRT;
	}

	{
		DXGI_SAMPLE_DESC SampleDesc;
		SampleDesc.Count = 1;
		SampleDesc.Quality = 0;
		Desc.SampleDesc = SampleDesc;
	}


	RootSignature = desc.RootSignature;
	BEAR_CHECK(RootSignature.empty() == false);
	RootSignaturePointer = static_cast<DX12RootSignature*>(RootSignature.get());
	Desc.pRootSignature = RootSignaturePointer->RootSignature.Get();

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
	streamDesc.pPipelineStateSubobjectStream = &Desc;
	streamDesc.SizeInBytes = sizeof(Desc);

	R_CHK(Factory->Device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&PipelineState)));
#endif
}

DX12PipelineMesh::~DX12PipelineMesh()
{
	PipelineMeshCounter--;
}

void* DX12PipelineMesh::QueryInterface(int Type)
{
	switch (Type)
	{
	case DX12Q_Pipeline:
		return reinterpret_cast<void*>(static_cast<DX12Pipeline*>(this));
	default:
		return nullptr;
	}
}

BearPipelineType DX12PipelineMesh::GetType()
{
	return PT_Mesh;
}

void DX12PipelineMesh::Set(
#ifdef DX12_1
#ifdef DX12UTIMATE
	ID3D12GraphicsCommandList6
#else
	ID3D12GraphicsCommandList4
#endif
#else
	ID3D12GraphicsCommandList
#endif
	
	* CommandList)
{
	CommandList->SetPipelineState(PipelineState.Get());
	CommandList->IASetPrimitiveTopology(TopologyType);
	RootSignaturePointer->Set(CommandList);
}
