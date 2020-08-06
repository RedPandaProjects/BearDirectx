#pragma once
class DX12DescriptorHeap :public BearRHI::BearRHIDescriptorHeap
{
public:
	DX12DescriptorHeap(const BearDescriptorHeapDescription& desc);
	virtual ~DX12DescriptorHeap();
	DX12AllocatorHeapItem UniSRVHeap;
	DX12AllocatorHeapItem SamplerHeap;
	D3D12_GPU_VIRTUAL_ADDRESS SRVs[16];
	D3D12_GPU_VIRTUAL_ADDRESS UAVs[16];

	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;

#ifdef RTX
	virtual void Set(ID3D12GraphicsCommandList4* CommandList);
#else
	virtual void SetGraphics(ID3D12GraphicsCommandList* CommandList);
	virtual void SetCompute(ID3D12GraphicsCommandList* CommandList);
#endif

	virtual void SetUniformBuffer(bsize slot, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> UniformBuffer, bsize offset = 0);
	virtual	void SetShaderResource(bsize slot, BearFactoryPointer<BearRHI::BearRHIShaderResource> ShaderResource, bsize offset = 0);
	virtual	void SetSampler(bsize slot, BearFactoryPointer<BearRHI::BearRHISampler> Sampler);
	virtual	void SetUnorderedAccess(bsize slot, BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> UnorderedAccess, bsize offset = 0);



	BearFactoryPointer<BearRHI::BearRHIUniformBuffer> UniformBuffers[16];
	bsize UniformBufferOffsets[16];
	bsize ShaderResourceOffsets[16];
	bsize UnorderedAccessOffsets[16];

	bsize CountBuffers;
	BearFactoryPointer<BearRHI::BearRHIShaderResource> ShaderResources[16];
	bsize CountSRVs;
	BearFactoryPointer<BearRHI::BearRHISampler> Samplers[16];
	bsize CountSamplers;
	BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> UnorderedAccess[16];
	bsize CountUAVs;

	bsize SlotBuffers[16];
	bsize SlotSRVs[16];
	bsize SlotSamplers[16];
	bsize SlotUAVs[16];
};