#pragma once
#include "AbstractList.h"

// �����: ���������������� ������
class List : public AbstractList
{
private:
    // ��������� ���� ������
    struct Node
    {
        void* data;       // ��������� �� ������
        size_t dataSize;  // ������ ������
        Node* next;       // ��������� �� ��������� ����

        Node(void* elem, size_t elemSize, Node* nextNode = nullptr) : next(nextNode), dataSize(elemSize)
        {
            data = elem;
        }

        ~Node()
        {
            // ������������� �������� ����������� �� ������� ����
        }
    };

    Node* _head;  // ��������� �� ������ ������
    int _size;    // ���������� ��������� � ������

public:
    // ����� ��������� ��� ������
    class ListIterator : public Container::Iterator
    {
    private:
        Node* _current;  // ������� ����
        Node* _prev;     // ���������� ���� (��� ��������)
        List* _list;     // ��������� �� ���������

    public:
        ListIterator(List* list, Node* current, Node* prev = nullptr)
            : _list(list), _current(current), _prev(prev) {}

        // ���������� ������� ������� � ��� ������
        void* getElement(size_t& size) override;

        // ��������� ������� ���������� ��������
        bool hasNext() override;

        // ��������� � ���������� ��������
        void goToNext() override;

        // ��������� ��������� ����������
        bool equals(Container::Iterator* right) override;

        // �������� ������� ����
        Node* getCurrent() { return _current; }

        // �������� ���������� ����
        Node* getPrev() { return _prev; }

        // ���������� ������� ���� (��� ����������� ���������� ����� ��������)
        void setCurrent(Node* node) { _current = node; }
    };

    // �����������
    List(MemoryManager& mem);

    // ����������
    ~List() override;

    // ���������� ������� Container
    int size() override;
    size_t max_bytes() override;
    Container::Iterator* find(void* elem, size_t size) override;
    Container::Iterator* newIterator() override;
    void remove(Container::Iterator* iter) override;
    void clear() override;
    bool empty() override;

    // ���������� ������� AbstractList
    int push_front(void* elem, size_t elemSize) override;
    void pop_front() override;
    void* front(size_t& size) override;
    int insert(Container::Iterator* iter, void* elem, size_t elemSize) override;

protected:
    // ��������������� ����� ��� ��������� ���������
    bool compareElements(void* elem1, size_t size1, void* elem2, size_t size2) const
    {
        return size1 == size2 && memcmp(elem1, elem2, size1) == 0;
    }
};