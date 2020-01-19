#pragma once
class DX12Factory :public BearRHI::BearRHIFactory
{
public:
	DX12Factory();
	virtual ~DX12Factory();
	inline bool Empty() const { return Device.Get() == 0; }
	virtual BearRHI::BearRHIViewport*  CreateViewport(void* Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearViewportDescription& Description);
	virtual BearRHI::BearRHIContext* CreateContext();
	virtual BearRHI::BearRHIShader* CreateShader(BearShaderType Type);
	virtual BearRHI::BearRHIVertexBuffer* CreateVertexBuffer();
	virtual BearRHI::BearRHIIndexBuffer* CreateIndexBuffer();
	virtual BearRHI::BearRHIUniformBuffer* CreateUniformBuffer();
	virtual BearRHI::BearRHIRootSignature* CreateRootSignature(const BearRootSignatureDescription& Description);
	virtual BearRHI::BearRHIDescriptorHeap* CreateDescriptorHeap(const BearDescriptorHeapDescription& Description);
	virtual BearRHI::BearRHIPipeline* CreatePipeline(const BearPipelineDescription& Description);
	virtual BearRHI::BearRHITexture2D* CreateTexture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, void* data = 0);
	virtual BearRHI::BearRHISampler* CreateSampler();
	static DXGI_FORMAT Translation(BearTexturePixelFormat format);
public:
	UINT SamplerDescriptorSize;
	UINT CbvSrvUavDescriptorSize;
	UINT RtvDescriptorSize;
	ComPtr<ID3D12RootSignature> RootSignature;
	ComPtr<ID3D12Device> Device;
	ComPtr<IDXGIFactory4> GIFactory;
#ifdef RTX
	ComPtr<ID3D12Device4> RTXDevice;

#endif
public:
	ComPtr<ID3D12GraphicsCommandList> CopyCommandList;
	void LockCommandList();
	void UnlockCommandList(ID3D12CommandQueue*CommandQueue=0);
public:
#ifdef RTX
	ComPtr<ID3D12GraphicsCommandList4> CommandList;
#else
	ComPtr<ID3D12GraphicsCommandList> CommandList;
#endif
private:
#ifdef RTX_SHADER_COMPILER
	ComPtr < IDxcCompiler> DxcCompiler;
	ComPtr < IDxcLibrary> DxcLibrary;
	IDxcIncludeHandler* DxcIncludeHandler;
#endif
	void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL&Level);
	BearVector<DXGI_MODE_DESC> m_GIVideoMode;
private:
	HANDLE m_Default_FenceEvent;
	uint64 m_Default_FenceValue;
	ComPtr<ID3D12Fence> m_Default_Fence;
	ComPtr<ID3D12CommandAllocator> m_Default_CommandAllocator;
	ComPtr<ID3D12CommandQueue> m_Default_CommandQueue;
	BearMutex m_Default_CommandMutex;
};