#include "DXPCH.h"
#include "DXInterface.h"


DXInterface::DXInterface()
{
	m_pixel_shader_current = 0;
	m_vertex_shader_current = 0;
	BearCore::bear_fill(m_renderTarget,  8, 0);
	for (bsize i = 0; i < 8; i++)
	{
		m_viewPorts[i].Height = 0;
		m_viewPorts[i].Width = 0;
		m_viewPorts[i].TopLeftX = 0;
		m_viewPorts[i].TopLeftY = 0;
		m_viewPorts[i].MaxDepth = 0;
		m_viewPorts[i].MinDepth = 0;
	}
	m_depthStencill = 0;
	m_count_texture_ps = 0;
	m_count_texture_vs = 0;
}

void DXInterface::clear()
{
}

void DXInterface::attachRenderTargetView(uint32 id, BearRHI::BearRHIViewPort * render_target)
{
	BEAR_ASSERT(id < 9);
	m_renderTarget[id] = (ID3D11RenderTargetView*)render_target->getRenderTarget();
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::attachRenderTargetView(uint32 id, BearRHI::BearRHIRenderTargetView * render_target)
{
	BEAR_ASSERT(id < 9);
	m_renderTarget[id] = (ID3D11RenderTargetView*)render_target->getRenderTarget();
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::attachDepthStencilView(BearRHI::BearRHIDepthStencilView * depthStencill)
{
	m_depthStencill = ((DXDepthStencilView*)depthStencill)->depthStencilView;
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::detachRenderTargetView(uint32 id)
{
	BEAR_ASSERT(id < 9);
	m_renderTarget[id] = (ID3D11RenderTargetView*)0;
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::detachDepthStencilView()
{
	m_depthStencill =0;
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::setViewport(uint32 id, float x, float y, float width, float height, float minDepth, float maxDepth)
{
	BEAR_ASSERT(id < 9);
	m_viewPorts[id].MaxDepth = maxDepth;
	m_viewPorts[id].MinDepth = minDepth;
	m_viewPorts[id].Width = width;
	m_viewPorts[id].Height = height;
	m_viewPorts[id].TopLeftX = x;
	m_viewPorts[id].TopLeftY = y;
	Factory->deviceContext->RSSetViewports(8, m_viewPorts);
}

void DXInterface::setPixelShader(BearRHI::BearRHIPixelShader * shader)
{
	m_count_texture_ps = 0;
	DXPixelShader*pixel = (DXPixelShader*)shader;
	m_pixel_shader_current = pixel;

	Factory->deviceContext->PSSetShader(pixel->shader, 0, 0);
}
void DXInterface::setVertexShader(BearRHI::BearRHIVertexShader * shader)
{
	m_count_texture_vs = 0;
	DXVertexShader*vertex = (DXVertexShader*)shader;
	m_vertex_shader_current = vertex;
	Factory->deviceContext->VSSetShader(vertex->shader, 0, 0);
}

void DXInterface::setIndexBuffer(BearRHI::BearRHIIndexBuffer * buffer)
{
	DXIndexBuffer*b = (DXIndexBuffer*)buffer;
	Factory->deviceContext->IASetIndexBuffer(b->buffer, DXGI_FORMAT_R32_UINT, 0);
}

void DXInterface::setVertexBuffer(BearRHI::BearRHIVertexBuffer * buffer,uint32 stride)
{
	DXVertexBuffer*b = (DXVertexBuffer*)buffer;
	UINT offset = 0;
	Factory->deviceContext->IASetVertexBuffers(0, 1,&b->buffer, &stride, &offset);
}

void DXInterface::setVertexInputLayout(BearRHI::BearRHIVertexInputLayout * buffer)
{
	DXVertexInputLayout*b = (DXVertexInputLayout*)buffer;
	Factory->deviceContext->IASetInputLayout(b->inputLayout);
}

void DXInterface::setBlendState(BearRHI::BearRHIBlendState * State, const  BearCore::BearColor&color)
{
	DXBlendState *state = (DXBlendState *)State;
	Factory->deviceContext->OMSetBlendState(state->BlendState, color.getFloat().array, 0xffffffff);
}

void DXInterface::setDepthStencilState(BearRHI::BearRHIDepthStencilState * State, uint32 StenciRef)
{
	DXDepthStencilState *state = (DXDepthStencilState *)State;
	Factory->deviceContext->OMSetDepthStencilState(state->DepthStencilState,StenciRef);
}

void DXInterface::setRasterizerState(BearRHI::BearRHIRasterizerState * State)
{
	DXRasterizerState *state = (DXRasterizerState *)State;
	Factory->deviceContext->RSSetState(state->RasterizerState);
}

void DXInterface::setItemInPixelShader(const char * name, BearRHI::BearRHITexture2D * texture2d, BearRHI::BearRHISamplerState * sampler)
{
	DXTexture2D *texture= (DXTexture2D *)texture2d;
	DXSamplerState *state = (DXSamplerState *)sampler;
	auto texture_resource = texture->getShaderResource();
	Factory->deviceContext->PSSetShaderResources(m_count_texture_ps, 1, &texture_resource);
	Factory->deviceContext->PSSetSamplers(m_count_texture_ps, 1, &state->SamplerState);
	m_count_texture_ps++;
}

void DXInterface::setItemInVertexShader(const char * name, BearRHI::BearRHITexture2D * texture2d, BearRHI::BearRHISamplerState * sampler)
{
	DXTexture2D *texture = (DXTexture2D *)texture2d;
	DXSamplerState *state = (DXSamplerState *)sampler;
	auto texture_resource = texture->getShaderResource();
	Factory->deviceContext->VSSetShaderResources(m_count_texture_ps, 1, &texture_resource);
	Factory->deviceContext->VSSetSamplers(m_count_texture_ps, 1, &state->SamplerState);
	m_count_texture_ps++;
}

void DXInterface::setItemInPixelShader(const char * name, float R)
{
	if (m_pixel_shader_current) *(float*)m_pixel_shader_current->shaderBuffer.getPointer(name, 4)=R;
}

void DXInterface::setItemInPixelShader(const char * name, const BearCore::BearVector2<float>& RG)
{
	if (m_pixel_shader_current) BearCore::bear_copy(m_pixel_shader_current->shaderBuffer.getPointer(name, 8), RG.array, 8);
}

void DXInterface::setItemInPixelShader(const char * name, const BearCore::BearVector3<float>& RGB)
{
	if (m_pixel_shader_current) BearCore::bear_copy(m_pixel_shader_current->shaderBuffer.getPointer(name, 12), RGB.array, 12);
}

void DXInterface::setItemInPixelShader(const char * name, const BearCore::BearVector4<float>& RGGBA)
{
	if (m_pixel_shader_current) BearCore::bear_copy(m_pixel_shader_current->shaderBuffer.getPointer(name, 16), RGGBA.array, 16);
}
void DXInterface::setItemInVertexShader(const char * name, float R)
{
	if (m_vertex_shader_current) *(float*)m_vertex_shader_current->shaderBuffer.getPointer(name, 4) = R;
}

void DXInterface::setItemInVertexShader(const char * name, const BearCore::BearVector2<float>& RG)
{
	if (m_vertex_shader_current) BearCore::bear_copy(m_vertex_shader_current->shaderBuffer.getPointer(name, 8), RG.array, 8);
}

void DXInterface::setItemInVertexShader(const char * name, const BearCore::BearVector3<float>& RGB)
{
	if (m_vertex_shader_current) BearCore::bear_copy(m_vertex_shader_current->shaderBuffer.getPointer(name, 12), RGB.array, 12);
}

void DXInterface::setItemInVertexShader(const char * name, const BearCore::BearVector4<float>& RGGBA)
{
	if (m_vertex_shader_current) BearCore::bear_copy(m_vertex_shader_current->shaderBuffer.getPointer(name, 16), RGGBA.array, 16);
}

void DXInterface::setItemInVertexShader(const char * name, const BearCore::BearMatrix & matrix)
{
	if (m_vertex_shader_current) BearCore::bear_copy(m_vertex_shader_current->shaderBuffer.getPointer(name, sizeof(float)*16), *matrix, sizeof(float) * 16);
}


void DXInterface::draw(bsize size, bsize possition, BearGraphics::BearDrawType mode)
{
	switch (mode)
	{
	case BearGraphics::DT_POINT:
		Factory->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		break;
	case BearGraphics::DT_LINE:
		Factory->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		break;
	case BearGraphics::DT_TRIANGLE:
		Factory->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	default:
		break;
	}
	if (m_pixel_shader_current)m_pixel_shader_current->shaderBuffer.PSSetBuffer();
	if (m_vertex_shader_current)m_vertex_shader_current->shaderBuffer.VSSetBuffer();
	Factory->deviceContext->DrawIndexed(static_cast<UINT>(size), static_cast<UINT>(possition), 0);
}


DXInterface::~DXInterface()
{
}
