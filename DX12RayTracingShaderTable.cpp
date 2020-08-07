#include "DX12PCH.h"
bsize RayTracingShaderTableCounter = 0;
DX12RayTracingShaderTable::DX12RayTracingShaderTable(const BearRayTracingShaderTableDescription& Description)
{
	RayTracingShaderTableCounter++;
#ifndef DX11
	DX12PipelineRayTracing * Pipeline = reinterpret_cast<DX12PipelineRayTracing*>(const_cast<BearRHI::BearRHIPipelineRayTracing*>(Description.Pipeline.get())->QueryInterface(DX12Q_RayTracingPipeline));
	BEAR_CHECK(Pipeline);

	ComPtr<ID3D12StateObjectProperties> StateObjectProperties;
	R_CHK(Pipeline->PipelineState.As(&StateObjectProperties));

	void * Identifier = StateObjectProperties->GetShaderIdentifier(*Description.Name);
	Size = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	CD3DX12_HEAP_PROPERTIES a(D3D12_HEAP_TYPE_UPLOAD );
	auto b = CD3DX12_RESOURCE_DESC::Buffer(Size);
	R_CHK(Factory->Device->CreateCommittedResource(
		&a,
		D3D12_HEAP_FLAG_NONE,
		&b,
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr,
		IID_PPV_ARGS(&Buffer)));
	void* Ptr = nullptr;
	R_CHK(Buffer->Map(0, nullptr, &Ptr));
	bear_copy(Ptr, Identifier, Size);
	Buffer->Unmap(0,nullptr);
#endif
}

DX12RayTracingShaderTable::~DX12RayTracingShaderTable()
{
	RayTracingShaderTableCounter--;
}
