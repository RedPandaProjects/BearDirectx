#pragma once
class DXShaderBuffer
{
public:
	DXShaderBuffer();
	void*getPointer(const char*nane,bsize size);
	void PSSetBuffer();
	void VSSetBuffer();
	~DXShaderBuffer();
private:
	void update();
	bsize getOffset(const char*nane,bsize size);
	bsize getTeleport(bsize offset,bsize size);
	void resize(bsize size);
	D3D11_BUFFER_DESC m_desc;
	ID3D11Buffer*m_directx_buffer;
	bsize m_offset;
	uint8* m_buffer;
	struct Item
	{
		Item(const char *t) { BearCore::bear_copy(text, t, BearCore::BearString::GetSize(t)+1); }
		Item() { text[0] = 0; }
		bool operator<(const char*t) const { return BearCore::bear_compare((void*)text,(void*) t, BearCore::BearString::GetSize(t)+1)<0; }
		bool operator==(const char*t) const { return BearCore::bear_compare((void*)text, (void*)t, BearCore::BearString::GetSize(t)+1)==0; }
		bool operator<(const Item&item) const { return BearCore::bear_compare((void*)text, (void*)item.text,64) < 0; }
		bool operator==(const Item&item)const { return BearCore::bear_compare((void*)text, (void*)item.text, 64)==0; }
		char text[64];
	};
	BearCore::BearMap<Item,bsize> m_offset_map;
};

