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

int List::insert(Iterator* iter, void* elem, size_t elemSize)
{
    ListIterator* Iter = (ListIterator*)iter;
    if (!Iter)
        return 1;
    Node* current = Iter->current;
    if (!current)
        return 1;
    Node* NewNode = (Node*)_memory.allocMem(sizeof(Node));
    if (!NewNode)
        return 1;
    memcpy(NewNode->data, elem, elemSize);
    NewNode->size = elemSize;
    NewNode->next = current->next;
    current->next = NewNode;
    count++;
}