#include "DX12PCH.h"
DX12RenderFactory *Factory = 0;
void RenderInitialize()
{
	Factory = BearCore::bear_new<DX12RenderFactory>();
	if (!Factory->Empty())
	{
		GRenderFactoty = Factory;
		BEAR_ASSERT(GRenderFactoty);
		return;
	}
	BearCore::bear_delete(Factory);
	GRenderFactoty = 0;

}
void RenderDestroy()
{
	if (GRenderFactoty)
	{
		BearCore::bear_delete(Factory);
		GRenderFactoty = 0;
	}
}
