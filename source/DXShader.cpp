#include "DXPCH.h"

uint16 GCountVertexShader = 0;
uint16 GCountPixelShader = 0;

DXVertexShader::DXVertexShader(void * data, bsize size)
{
	Factory->device->CreateVertexShader(data, size, NULL, &shader);
	GCountVertexShader++;
}

DXVertexShader::~DXVertexShader()
{
	if(shader)
	shader->Release();
	GCountVertexShader--;
}

DXPixelShader::DXPixelShader(void * data, bsize size)
{
	Factory->device->CreatePixelShader(data, size, NULL, &shader);
	GCountPixelShader++;
}

DXPixelShader::~DXPixelShader()
{
	if (shader)
		shader->Release();
	GCountPixelShader--;
}
