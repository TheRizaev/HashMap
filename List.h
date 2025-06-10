#pragma once
#include "AbstractList.h"
#include <vector>

typedef struct Node
{
    void* data;
    size_t size;
    Node* next;
};
class List : public AbstractList
{
private:
    Node* head;
    int count;
public:
    List(MemoryManager& mem) : AbstractList(mem), head(NULL) {}
    ~List() { clear(); }
    class ListIterator : public Iterator
    {
    public:
        Node* current;
        Node* previous;
        //ListIterator(Node* begin) : current(begin) {}
        ListIterator() : current(NULL), previous(NULL) {}
        ListIterator(Node* begin, Node* prev = nullptr) : current(begin), previous(prev) {}
        ~ListIterator() {}
        // ���������� ���� ��������� �� �������, �� ������� ��������� �������� � ������ ������.
        // ������ ���������� ������ ������.
        // ���� �������� ���������� �� ������� ���������� (��������, ������ ��������� �������), ���������� NULL.
        void* getElement(size_t& size)
        {
            if (!current)
            {
                size = 0;
                return NULL;
            }
            int* value = (int*)current->data;
            size = current->size;
            return current->data;
        }

        // ���������� true, ���� ���� ��������� �������, ����� false.
        bool hasNext()
        {
            if (current)
                return current->next;
            else
                return false;
        }

        // ������� � ���������� ��������.
        void goToNext()
        {
            if (current)
            {
                //int* value = (int*)current->data;
                previous = current;
                current = current->next;
                //value = (int*)current->data;
            }
        }

        // �������� �� ��������� ����������
        bool equals(Iterator* right)
        {
            ListIterator* Iter = (ListIterator*)right;
            if (!Iter)
                return false;
            return Iter->current == current;
        }
    };
    // ���������� �������� � ������ ����������.
    // � ������ ��������� ���������� ������� ���������� �������� 0, � ������ ������� 1.
    int push_front(void* elem, size_t elemSize);

    // �������� �������� �� ������ ����������.
    void pop_front();

    // ������� ���� ���������� ��������� �� �������, ����������� � ������ ����������.
    // � �� ���� ���������� ������ ������
    void* front(size_t& size);

    // ���������� �������� � �������, �� ������� ��������� �������� iter.
    // � ������ ��������� ���������� ������� ���������� �������� 0, � ������ ������� 1.
    int insert(Iterator* iter, void* elem, size_t elemSize);


    // ������� ���������� ��������, ������ ���������� ��������� � ����������.
    int size() { return count; }
    //���������
    // ������� ���������� ��������, ������ ������������ ����������� ���������� � ������.
    size_t max_bytes() { return _memory.maxBytes(); }

    Iterator* find(void* elem, size_t size)
    {
        ListIterator* Iter = new ListIterator();
        Iter->current = head;

        while (Iter->current != nullptr)
        {
            if (Iter->current->size == size &&
                memcmp(Iter->current->data, elem, size) == 0)
            {
                return Iter;
            }
            Iter->previous = Iter->current;
            Iter->current = Iter->current->next;
        }

        delete Iter;
        return nullptr;
    }

    //std::vector<Iterator*> findAll(void* elem, size_t size)
    //{
    //    std::vector<Iterator*> results;
    //    Node* tmp = head;
    //    while (tmp != nullptr)
    //    {
    //        if (tmp->size == size && memcmp(tmp->data, elem, size) == 0)
    //        {
    //            results.push_back(new ListIterator(tmp));
    //        }
    //        tmp = tmp->next;
    //    }
    //    return results;
    //}



    // ������� ������� � ������������ ������ ��������, ����������� �� ������ �������
    // ����������. ���� ��������� ������, ������������ ������� ���������.
    // �������� ����� ��������� ������ �������� ������������� � ������� ��������� delete.
    Iterator* newIterator()
    {
        if (!head)
        {
            return NULL;
        }
        ListIterator* Iter = new ListIterator();
        //ListIterator* Iter = (ListIterator*)_memory.allocMem(sizeof(ListIterator));
        Iter->current = head;
        return Iter;
    }

    //���������� ?
    // �������� �������� �� �������, �� ������� ��������� �������� iter.
    // ����� �������� �������� ��������� �� ��������� �� ��������� �������.
    void remove(Iterator* iter)
    {
        ListIterator* Iter = (ListIterator*)iter;
        if (!(!Iter || head)) return;
        Node* current = head;
        if (Iter && current == Iter->current)
        {
            head = current->next;
            //int* value = (int*)head->data;
            Iter->goToNext();
            _memory.freeMem(current->data);
            _memory.freeMem(current);
            count--;
            return;
        }
        Node* tmp = current;
        while (current->next && current->next != Iter->current) {
            tmp = current;
            current = current->next;
        }
        if (current->next) {
            Node* temp = current->next;
            current->next = temp->next;
            Iter->goToNext();
            _memory.freeMem(temp->data);
            _memory.freeMem(temp);
            count--;
        }
    }


    // �������� ���� ��������� �� ����������.
    void clear()
    {
        if (!head)
            return;
        while (head)
        {
            Node* temp = head;
            head = head->next;
            _memory.freeMem(temp->data);
            _memory.freeMem(temp);
        }
        count = 0;
    }

    // ���� ��������� ���� ���������� true, ����� false
    bool empty()
    {
        return head == NULL;
    }
    void print()
    {
        Node* temp = head;
        while (temp)
        {

            printf("%d\n", *(int*)temp->data);
            temp = temp->next;
        }
    }
};

