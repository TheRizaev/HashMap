#include "List.h"
#include <cstring>

// ���������� ������� ��������� ������
void* List::ListIterator::getElement(size_t& size)
{
    if (_current == nullptr)
        return nullptr;

    size = _current->dataSize;
    return _current->data;
}

bool List::ListIterator::hasNext()
{
    return _current != nullptr && _current->next != nullptr;
}

void List::ListIterator::goToNext()
{
    if (_current != nullptr)
    {
        _prev = _current;
        _current = _current->next;
    }
}

bool List::ListIterator::equals(Container::Iterator* right)
{
    List::ListIterator* rightIter = dynamic_cast<List::ListIterator*>(right);
    if (rightIter == nullptr)
        return false;

    return _current == rightIter->_current;
}

// ���������� ������� ������
List::List(MemoryManager& mem) : AbstractList(mem), _head(nullptr), _size(0) {}

List::~List()
{
    clear();
}

int List::size()
{
    return _size;
}

size_t List::max_bytes()
{
    return _memory.maxBytes();
}

Container::Iterator* List::find(void* elem, size_t size)
{
    Node* current = _head;
    Node* prev = nullptr;

    while (current != nullptr)
    {
        if (compareElements(current->data, current->dataSize, elem, size))
            return new ListIterator(this, current, prev);

        prev = current;
        current = current->next;
    }

    return nullptr;
}

Container::Iterator* List::newIterator()
{
    if (_head == nullptr)
        return nullptr;

    return new ListIterator(this, _head);
}

void List::remove(Container::Iterator* iter)
{
    ListIterator* listIter = dynamic_cast<ListIterator*>(iter);
    if (listIter == nullptr)
        return;

    Node* current = listIter->getCurrent();
    Node* prev = listIter->getPrev();

    if (current == nullptr)
        return;

    // ��������� ��������� �� ��������� ���� ��� ���������� ���������
    Node* nextNode = current->next;

    if (prev == nullptr) // ������� ������ �������
    {
        _head = nextNode;
    }
    else
    {
        prev->next = nextNode;
    }

    // ����������� ������ ������ ����
    _memory.freeMem(current->data);

    // ����������� ������ ������ ����
    delete current;
    _size--;

    // ��������� ��������� ��������, ��������� ����� ����� setCurrent
    listIter->setCurrent(nextNode);
}

void List::clear()
{
    Node* current = _head;
    while (current != nullptr)
    {
        Node* next = current->next;
        _memory.freeMem(current->data);
        delete current;
        current = next;
    }
    _head = nullptr;
    _size = 0;
}

bool List::empty()
{
    return _head == nullptr;
}

int List::push_front(void* elem, size_t elemSize)
{
    try
    {
        // �������� ������ ��� ������
        void* newData = _memory.allocMem(elemSize);
        if (newData == nullptr)
            return 1;

        // �������� ������
        memcpy(newData, elem, elemSize);

        // ������� ����� ����
        Node* newNode = new Node(newData, elemSize, _head);
        _head = newNode;
        _size++;

        return 0;
    }
    catch (...)
    {
        return 1;
    }
}

void List::pop_front()
{
    if (_head == nullptr)
        return;

    Node* oldHead = _head;
    _head = _head->next;

    _memory.freeMem(oldHead->data);
    delete oldHead;
    _size--;
}

void* List::front(size_t& size)
{
    if (_head == nullptr)
        return nullptr;

    size = _head->dataSize;
    return _head->data;
}

int List::insert(Container::Iterator* iter, void* elem, size_t elemSize)
{
    ListIterator* listIter = dynamic_cast<ListIterator*>(iter);
    if (listIter == nullptr)
        return 1;

    Node* current = listIter->getCurrent();
    Node* prev = listIter->getPrev();

    try
    {
        // �������� ������ ��� ������
        void* newData = _memory.allocMem(elemSize);
        if (newData == nullptr)
            return 1;

        // �������� ������
        memcpy(newData, elem, elemSize);

        // ������� ����� ����
        Node* newNode = new Node(newData, elemSize);

        if (prev == nullptr) // ������� � ������
        {
            newNode->next = _head;
            _head = newNode;
        }
        else // ������� � �������� ��� �����
        {
            newNode->next = current;
            prev->next = newNode;
        }

        _size++;
        return 0;
    }
    catch (...)
    {
        return 1;
    }
}