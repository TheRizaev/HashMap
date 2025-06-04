#pragma once
#include "AbstractList.h"
#include "Mem.h"
#include "MemoryManager.h"
#include <iostream>

typedef struct List_struct
{
	List_struct* ptr;
	void* elem;
	size_t size;
}list_struct;

class List : public AbstractList
{
	int ElementCount = 0;
	size_t MaxBytes = 0;
	list_struct* head = nullptr;
public:
	List(MemoryManager& mem) : AbstractList(mem) {}
	int push_front(void* element, size_t size);
	void pop_front();
	void* front(size_t& sz);
	int insert(Iterator* iter, void* elem, size_t size);
	int size();
	size_t max_bytes();
	Iterator* find(void* elem, size_t size);
	Iterator* newIterator();
	void remove(Iterator* iter);
	void clear();
	bool empty();
	class ListIterator : public	Iterator
	{
		list_struct* CurrStruct = nullptr;
		list_struct* PrevStruct = nullptr;
	public:
		friend class List;
		void* getElement(size_t& sz);
		bool hasNext();
		void goToNext();
		bool equals(Iterator* right);
	};
	friend class ListIterator;
	~List();
};