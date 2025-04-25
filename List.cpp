#include <iostream>
#include "AbstractList.h"
#include "Mem.h"
#include "MemoryManager.h"
#include "List.h"
#include "Table.h"

int List::push_front(void* element, size_t size)
{
	list_struct* newhead = (list_struct*)_memory.allocMem(sizeof(list_struct));
	newhead->elem = _memory.allocMem(size);
	std::memcpy(newhead->elem, element, size);
	newhead->size = size;
	newhead->ptr = head;
	head = newhead;
	ElementCount++;
	return 0;
}
void List::pop_front()
{
	if (!head)
		return;
	else if (!head->ptr)
	{
		_memory.freeMem(head->elem);
		_memory.freeMem(head);
		ElementCount--;
		head = nullptr;
		return;
	}
	list_struct* newhead = head->ptr;
	_memory.freeMem(head->elem);
	_memory.freeMem(head);
	ElementCount--;
	head = newhead;
}
void* List::front(size_t& sz)
{
	if (!head)
	{
		sz = 0;
		return nullptr;
	}
	sz = head->size;
	return head->elem;
}
int List::insert(Iterator* iter, void* elem, size_t size)
{
	if (!iter)
	{
		push_front(elem, size);
		return 0;
	}
	list_struct* prev = ((ListIterator*)iter)->PrevStruct;
	list_struct* curr = ((ListIterator*)iter)->CurrStruct;
	list_struct* newstruct = (list_struct*)_memory.allocMem(sizeof(List_struct));
	newstruct->elem = _memory.allocMem(size);
	std::memcpy(newstruct->elem, elem, size);
	newstruct->size = size;
	ElementCount++;
	if (!prev)
	{
		newstruct->ptr = head;
		head = newstruct;
		((ListIterator*)iter)->PrevStruct = nullptr;
		((ListIterator*)iter)->CurrStruct = head;
		return 0;
	}
	else if (!curr)
	{
		prev->ptr = newstruct;
		newstruct->ptr = nullptr;
		((ListIterator*)iter)->CurrStruct = newstruct;
		return 0;
	}
	newstruct->ptr = prev->ptr;
	prev->ptr = newstruct;
	((ListIterator*)iter)->CurrStruct = newstruct;
	return 0;
}
int List::size()
{
	return ElementCount;
}
size_t List::max_bytes()
{
	return MaxBytes;
}
Container::Iterator* List::find(void* elem, size_t size)
{
	if (!head)
		return nullptr;
	int sz = List::size();
	list_struct* step = head, * prev_step = nullptr;
	for (int i = 0; i < sz; i++)
	{
		if (step->size == size && !memcmp(step->elem, elem, size))
		{
			ListIterator* Iterator = new ListIterator;
			Iterator->CurrStruct = step;
			Iterator->PrevStruct = prev_step;
			return Iterator;
		}
		prev_step = step;
		step = step->ptr;
	}
	return nullptr;
}
Container::Iterator* List::newIterator()
{
	if (!head)
		return nullptr;
	ListIterator* Iterator = new ListIterator;
	Iterator->CurrStruct = head;
	return Iterator;
}
void List::remove(Iterator* iter)
{
	list_struct* prev = ((ListIterator*)iter)->PrevStruct;
	list_struct* curr = ((ListIterator*)iter)->CurrStruct;
	if ((!prev && !curr) || !curr)
		return;
	if (!prev)
	{
		iter->goToNext();
		pop_front();
		((ListIterator*)iter)->PrevStruct = nullptr;
		return;
	}
	ElementCount--;
	if (!curr->ptr)
	{
		_memory.freeMem(curr->elem);
		_memory.freeMem(curr);
		prev->ptr = nullptr;
		((ListIterator*)iter)->CurrStruct = nullptr;
		return;
	}
	iter->goToNext();
	prev->ptr = curr->ptr;
	_memory.freeMem(curr->elem);
	_memory.freeMem(curr);
	((ListIterator*)iter)->PrevStruct = prev;
}
void List::clear()
{
	if (!head)
		return;
	else if (!head->ptr)
	{
		_memory.freeMem(head->elem);
		_memory.freeMem(head);
		ElementCount = 0;
		head = nullptr;
		return;
	}
	size_t sz = size();
	list_struct* next;
	for (int i = 0; i < sz - 1; i++)
	{
		next = head->ptr;
		_memory.freeMem(head->elem);
		_memory.freeMem(head);
		head = next;
	}
	_memory.freeMem(head->elem);
	_memory.freeMem(head);
	ElementCount = 0;
	head = nullptr;
}
bool List::empty()
{
	if (!head)
		return true;
	return false;
}
void* List::ListIterator::getElement(size_t& sz)
{
	if (!CurrStruct)
		return nullptr;
	sz = CurrStruct->size;
	return CurrStruct->elem;
}
bool List::ListIterator::hasNext()
{
	if (!CurrStruct)
		return false;
	else if (!CurrStruct->ptr)
		return false;
	return true;
}
void List::ListIterator::goToNext()
{
	if (CurrStruct)
	{
		PrevStruct = CurrStruct;
		CurrStruct = CurrStruct->ptr;
	}
}
bool List::ListIterator::equals(Iterator* right)
{
	if (!right)
		return false;
	ListIterator* IterRight = (ListIterator*)right;
	if (CurrStruct == IterRight->CurrStruct)
		return true;
	else if (!CurrStruct || !IterRight->CurrStruct)
		return false;
	return false;
}
List::~List()
{
	this->clear();
}