#include "DXPCH.h"


uint16 GCountVertexInputLayout = 0;



DXVertexInputLayout::DXVertexInputLayout(const BearGraphics::BearVertexInputLayoutInitializer & initializer, void * data, bsize size)
{
	BearCore::BearVector<D3D11_INPUT_ELEMENT_DESC> m_input_elements;
	BearCore::BearVector<BearCore::BearMemoryRef<bchar8>> m_ptr_stack;
	UINT Offset = 0;
	for (bsize i = 0; i < initializer.count(); i++)
	{
		D3D11_INPUT_ELEMENT_DESC item;
		m_ptr_stack.push_back(BearCore::BearEncoding::ToANSI( initializer.getName(i)));

		item.SemanticName = *m_ptr_stack.back();
		item.SemanticIndex = 0;
		item.Format = DXFactory::TranslateInputLayoutElement(initializer.getType(i));
		item.InputSlot = 0;
		item.AlignedByteOffset = Offset;
		item.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		item.InstanceDataStepRate = 0;
		Offset +=static_cast<UINT>( initializer.GetSizeElement(initializer.getType(i)));
		m_input_elements.push_back(item);
	}
	R_CHK(Factory->device->CreateInputLayout(&m_input_elements[0], static_cast<UINT>(m_input_elements.size()), data,size, &inputLayout));
	GCountVertexInputLayout++;
}

DXVertexInputLayout::~DXVertexInputLayout()
{
	GCountVertexInputLayout--;
	inputLayout->Release();
}
