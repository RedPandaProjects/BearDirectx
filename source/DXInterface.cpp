#include "DXPCH.h"
#include "DXInterface.h"


DXInterface::DXInterface()
{
	BearCore::bear_fill(m_renderTarget,  8, 0);
	m_depthStencill = 0;
}

void DXInterface::Clear()
{
}

void DXInterface::AttachRenderTargetView(uint32 id, BearRHI::BearRHIViewport * render_target)
{
	BEAR_ASSERT(id < 9);
	m_renderTarget[id] = (ID3D11RenderTargetView*)render_target->GetRenderTarget();
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::AttachRenderTargetView(uint32 id, BearRHI::BearRHIRenderTargetView * render_target)
{
	BEAR_ASSERT(id < 9);
	m_renderTarget[id] = (ID3D11RenderTargetView*)render_target->GetRenderTarget();
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::AttachDepthStencilView(BearRHI::BearRHIDepthStencilView * depthStencill)
{
	m_depthStencill = ((DXDepthStencilView*)depthStencill)->depthStencilView;
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::DetachRenderTargetView(uint32 id)
{
	BEAR_ASSERT(id < 9);
	m_renderTarget[id] = (ID3D11RenderTargetView*)0;
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::DetachDepthStencilView()
{
	m_depthStencill =0;
	Factory->deviceContext->OMSetRenderTargets(8, m_renderTarget, m_depthStencill);
}

void DXInterface::SetViewport( float x, float y, float width, float height, float minDepth, float maxDepth)
{

	D3D11_VIEWPORT view_port;
	view_port.MaxDepth = maxDepth;
	view_port.MinDepth = minDepth;
	view_port.Width = width;
	view_port.Height = height;
	view_port.TopLeftX = x;
	view_port.TopLeftY = y;
	Factory->deviceContext->RSSetViewports(1, &view_port);

}

void DXInterface::SetScissor( float x, float y, float x1, float y1)
{
	D3D11_RECT rect;
	rect.left = static_cast<LONG>(x);
	rect.right = static_cast<LONG>(x1);
	rect.top = static_cast<LONG>(y);
	rect.bottom = static_cast<LONG>(y1);
	Factory->deviceContext->RSSetScissorRects(1, &rect);
}

void DXInterface::SetPixelShader(BearRHI::BearRHIPixelShader * shader)
{
	DXPixelShader*pixel = (DXPixelShader*)shader;
	Factory->deviceContext->PSSetShader(pixel->shader, 0, 0);
}
void DXInterface::SetVertexShader(BearRHI::BearRHIVertexShader * shader)
{
	DXVertexShader*vertex = (DXVertexShader*)shader;
	Factory->deviceContext->VSSetShader(vertex->shader, 0, 0);
}

void DXInterface::SetIndexBuffer(BearRHI::BearRHIIndexBuffer * buffer)
{
	DXIndexBuffer*b = (DXIndexBuffer*)buffer;
	Factory->deviceContext->IASetIndexBuffer(b->buffer, DXGI_FORMAT_R32_UINT, 0);
}

void DXInterface::SetVertexBuffer(BearRHI::BearRHIVertexBuffer * buffer)
{
	DXVertexBuffer*b = (DXVertexBuffer*)buffer;
	UINT offSet = 0;
	Factory->deviceContext->IASetVertexBuffers(0, 1,&b->buffer, &m_stride, &offSet);
}

void DXInterface::SetVertexState(BearRHI::BearRHIVertexState * buffer)
{
	DXVertexState*b = (DXVertexState*)buffer;
	m_stride = b->Stride;
	Factory->deviceContext->IASetInputLayout(b->InputLayout);
}

void DXInterface::SetBlendState(BearRHI::BearRHIBlendState * State, const  BearCore::BearColor&color)
{
	DXBlendState *state = (DXBlendState *)State;
	Factory->deviceContext->OMSetBlendState(state->BlendState, color.GetFloat().array, 0xffffffff);
}

void DXInterface::SetDepthStencilState(BearRHI::BearRHIDepthStencilState * State, uint32 StenciRef)
{
	DXDepthStencilState *state = (DXDepthStencilState *)State;
	Factory->deviceContext->OMSetDepthStencilState(state->DepthStencilState,StenciRef);
}

void DXInterface::SetRasterizerState(BearRHI::BearRHIRasterizerState * State)
{
	DXRasterizerState *state = (DXRasterizerState *)State;
	Factory->deviceContext->RSSetState(state->RasterizerState);
}

void DXInterface::SetPixelShaderConstants(bsize slot, BearRHI::BearRHIShaderConstants * constants)
{
	Factory->deviceContext->PSSetConstantBuffers(static_cast<UINT>(slot), 1, &((DXShaderConstants *)constants)->Buffer);
}

void DXInterface::SetPixelShaderResource(bsize slot, BearRHI::BearRHITexture2D * texture2d, BearRHI::BearRHISamplerState * sampler)
{
	Factory->deviceContext->PSSetShaderResources(static_cast<UINT>(slot), 1, &((DXTexture2D *)texture2d)->ShaderTexture);
	Factory->deviceContext->PSSetSamplers(static_cast<UINT>(slot), 1, &((DXSamplerState *)sampler)->SamplerState);

}

void DXInterface::SetVertexShaderConstants(bsize slot, BearRHI::BearRHIShaderConstants * constants)
{
	Factory->deviceContext->VSSetConstantBuffers(static_cast<UINT>(slot), 1, &((DXShaderConstants *)constants)->Buffer);
}

void DXInterface::SetVertexShaderResource(bsize slot, BearRHI::BearRHITexture2D * texture2d, BearRHI::BearRHISamplerState * sampler)
{
	Factory->deviceContext->VSSetShaderResources(static_cast<UINT>(slot), 1, &((DXTexture2D *)texture2d)->ShaderTexture);
	Factory->deviceContext->VSSetSamplers(static_cast<UINT>(slot), 1, &((DXSamplerState *)sampler)->SamplerState);
}


void DXInterface::Draw(bsize size, bsize possition, BearGraphics::BearDrawType mode)
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
	Factory->deviceContext->DrawIndexed(static_cast<UINT>(size), static_cast<UINT>(possition), 0);
}


DXInterface::~DXInterface()
{
}
