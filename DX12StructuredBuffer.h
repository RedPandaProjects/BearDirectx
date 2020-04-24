#pragma once
class DX12StructuredBuffer:public BearRHI::BearRHIStructuredBuffer,public DX12ShaderResource
{
public:
	DX12StructuredBuffer(bsize Size,void* Data);
	virtual ~DX12StructuredBuffer();
	ComPtr<ID3D12Resource> StructuredBuffer;

	virtual bool SetAsSRV(D3D12_GPU_VIRTUAL_ADDRESS& ADDRESS, bsize offset);
	virtual void* QueryInterface(int Type);
private:
	bsize m_size;
};

