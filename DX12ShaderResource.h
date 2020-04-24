#pragma once
class DX12ShaderResource :public virtual BearRHI::BearRHIShaderResource
{
public:
	virtual bool SetAsSRV(D3D12_GPU_VIRTUAL_ADDRESS& ADDRESS,bsize offset) { return false; }
	virtual bool SetAsSRV(D3D12_CPU_DESCRIPTOR_HANDLE& HEAP) { return false; }
protected:
	D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
};