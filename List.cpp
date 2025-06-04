#include "List.h"
#include "stdlib.h"

int List::push_front(void* elem, size_t elemSize) {
    Node* newNode = (Node*)_memory.allocMem(sizeof(Node));
    if (!newNode) return 1;

    newNode->data = _memory.allocMem(elemSize);
    if (!newNode->data) {
        _memory.freeMem(newNode);
        return 1;
    }

    memcpy(newNode->data, elem, elemSize);
    newNode->size = elemSize;
    newNode->next = head;
    head = newNode;
    count++;
    return 0;
}
void List::pop_front()
{
	if (!head)
		return;
	Node* temp = head;
	head = head->next;
    _memory.freeMem(temp->data);
    _memory.freeMem(temp);
    count--;
}
void* List::front(size_t& size)
{
	if (head)
	{
		size = head->size;
		return head->data;
	}
    size = 0;
    return NULL;
}
// Добавление элемента в позицию, на которую указывает итератор iter.
// В случае успешного добавления функция возвращает значение 0, в случае неудачи 1.
int List::insert(Iterator* iter, void* elem, size_t elemSize)
{
    ListIterator* Iter = (ListIterator*)iter;
        if (!Iter || !elem || elemSize == 0)
            return 1;

        Node* prev = Iter->previous;
        Node* current = Iter->current;


        Node* newNode = (Node*)_memory.allocMem(sizeof(Node));
        if (!newNode)
            return 1;

        newNode->data = _memory.allocMem(elemSize);
        if (!newNode->data)
            return 1;

        memcpy(newNode->data, elem, elemSize);
        newNode->size = elemSize;

        if (!prev) //значит итератор смотрит на head
        {
            newNode->next = current;
            head = newNode;
            Iter->previous = head;
            Iter->current = current;
        }
        else
        {
            newNode->next = current;
            prev->next = newNode;
            Iter->previous = newNode;
            Iter->current = current;
        }
        count++;
        return 0;
}
