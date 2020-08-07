#pragma once
class DX12RayTracingShaderTable :public BearRHI::BearRHIRayTracingShaderTable
{
public:
	DX12RayTracingShaderTable(const BearRayTracingShaderTableDescription& Description);
	virtual ~DX12RayTracingShaderTable();
	ComPtr<ID3D12Resource> Buffer;
	bsize Size;
};