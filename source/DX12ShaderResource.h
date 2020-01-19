#pragma once
class DX12ShaderResource :public virtual BearRHI::BearRHIShaderResource
{
public:
	virtual void SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE*HEAP)=0;
protected:
	D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
};