#pragma once
#include "BearRHI.h"
#pragma warning(disable:4005)
#include "directx/D3D11.h"
#include "directx/D3DX11.h"
#include "directx/DXGI.h"
#include "directx/DxErr.h"
#include "DXFactory.h"
#include "DXTexture2D.h"
#include "DXTexture2DArray.h"
#include "DXRenderTergetView.h"
#include "DXDepthStencilView.h"
#include "DXViewPort.h"
#include "DXShader.h"
#include "DXShaderCompiler.h"
#include "DXBuffer.h"
#include "DXInterface.h"
#include "DXStats.h"
#include "DXShaderConstants.h"
#include "DXDefaultManager.h"

#define R_CHK(a) BEAR_ASSERT(SUCCEEDED(a))
extern DXFactory *Factory;