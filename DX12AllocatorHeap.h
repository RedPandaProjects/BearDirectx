#pragma once
struct  DX12AllocatorHeapItem
{
	DX12AllocatorHeapItem() :Id(0), Size(0) {}
	ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
	bsize Id;
	bsize Size;
};

template<D3D12_DESCRIPTOR_HEAP_TYPE heap_type, bsize elements_in_heap=128 >
class  DX12AllocatorHeap
{
public:
	DX12AllocatorHeap() {}
	inline ~DX12AllocatorHeap()
	{

	}
	inline void clear()
	{

		for (bsize i = 0; i < HeapsOfAddres.size(); i++)
		{
			HeapsOfAddres[i]->DescriptorHeap.Reset();
			bear_delete(HeapsOfAddres[i]);
#ifdef DEBUG
			BEAR_CHECK(HeapsOfAddres[i]->MaxSize == elements_in_heap);
#endif
		}

	}
	inline DX12AllocatorHeapItem allocate(bsize count_element)
	{
		BEAR_CHECK(elements_in_heap >=count_element);
		DX12AllocatorHeapItem result;
		Heap* heap = 0;
		if(HeapsOfMaxSize.size())
		{
			auto item = std::lower_bound(HeapsOfMaxSize.begin(), HeapsOfMaxSize.end(), count_element, [](Heap* a1, bsize a2)->bool {return a1->MaxSize < a2; });
			if (item == HeapsOfMaxSize.end())
			{
				heap = create_block();
			}
			else
			{

				heap = *item;
			}
		
		}
		else
		{
			heap = create_block();
		}
		if (heap->MaxSize < count_element)
		{
			heap = create_block();
		}
		result.DescriptorHeap = heap->DescriptorHeap;
		bsize id = find_id(count_element, heap);
		result.Id = id;
		result.Size = count_element;
		memset(&heap->BlockInfo[id], 1, count_element);
		(heap)->MaxSize = get_max_size(heap);
		bear_sort(HeapsOfMaxSize.begin(), HeapsOfMaxSize.end(), [](Heap* a1, Heap* a2)->bool {return a1->MaxSize < a2->MaxSize; });

		return result;
	}
	inline void free(DX12AllocatorHeapItem& Item)
	{
		if (Item.Size == 0)return;
		auto item = std::lower_bound(HeapsOfAddres.begin(), HeapsOfAddres.end(), Item, [](const Heap* a1,const DX12AllocatorHeapItem& a2)->bool {return a1->DescriptorHeap.Get() < a2.DescriptorHeap.Get(); });
		BEAR_CHECK(item != HeapsOfAddres.end());
		BEAR_CHECK((*item)->DescriptorHeap.Get() == Item.DescriptorHeap.Get());
		memset(&(*item)->BlockInfo[Item.Id],0 , Item.Size);
		(*item)->MaxSize = get_max_size(*item);
		bear_sort(HeapsOfMaxSize.begin(), HeapsOfMaxSize.end(), [](Heap* a1, Heap* a2)->bool {return a1->MaxSize < a2->MaxSize; });
		Item.DescriptorHeap.Reset();
		Item.Id = 0;
		Item.Size = 0;
	}
private:
	struct Heap
	{
		ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
		uint8 BlockInfo[elements_in_heap];
		bsize MaxSize;
	};
	inline Heap* create_block()
	{
		Heap* heap = bear_new<Heap>();

		{
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = static_cast<UINT>(elements_in_heap);
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.Type = heap_type;
			R_CHK(Factory->Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&heap->DescriptorHeap)));
		}
		memset(heap->BlockInfo,0, elements_in_heap);
		heap->MaxSize = elements_in_heap;
		auto item1 = std::lower_bound(HeapsOfAddres.begin(), HeapsOfAddres.end(), heap, [](Heap*a1, Heap* a2)->bool {return a1->DescriptorHeap.Get() < a2->DescriptorHeap.Get(); });
		HeapsOfAddres.insert(item1, heap);
		auto item2 = std::lower_bound(HeapsOfMaxSize.begin(), HeapsOfMaxSize.end(), elements_in_heap, [](Heap* a1, bsize a2)->bool {return a1->MaxSize < a2; });
		HeapsOfMaxSize.insert(item2, heap);
		return heap;

	}
	inline bsize get_size(Heap* heap, bsize id)
	{
		bsize count = 0;
		for (bsize i = id; i < elements_in_heap; i++)
		{
			if (heap->BlockInfo[i]==0)
			{
				count++;
			}
			else
			{
				return count;
			}
		}
		return count;
	}
	inline bsize find_id(bsize size,Heap* heap)
	{
		for (bsize i = 0; i < elements_in_heap;)
		{
			bsize fsize = get_size(heap, i);
			if (fsize)
			{
				if (fsize >= size)
				{
					return i;
				}
				i += fsize;
			}
			else
			{
				i++;
			}
		}
		BEAR_CHECK(0);
		return 0;
	}
	inline bsize get_max_size(Heap* heap)
	{
		bsize MaxSize = 0;
		for (bsize i = 0; i < elements_in_heap;)
		{
			bsize fsize = get_size(heap, i);
			if (fsize)
			{
				MaxSize = BearMath::max(fsize, MaxSize);
				i += fsize;
			}
			else
			{
				i++;
			}
	
		}

		return MaxSize;
	}
	BearVector<Heap*> HeapsOfAddres;
	BearVector<Heap*> HeapsOfMaxSize;
};