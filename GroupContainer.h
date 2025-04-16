#pragma once
#include "Container.h"
#include "AbstractList.h"
#include "List.h"
#include "MemoryManager.h"

// GroupContainer - ������������� �����, ����� ��� ��������� � �������
// ��������� ������� ��������, ��������� ���������� ������ List
class GroupContainer : public Container
{
protected:
    AbstractList* _list;  // ���������� ������ ��� �������� ���������

public:
    // ����������� - ��������� ������ �� �������� ������
    GroupContainer(MemoryManager& mem) : Container(mem)
    {
        _list = new List(mem);
    }

    // ����������
    virtual ~GroupContainer()
    {
        if (_list)
            delete _list;
    }

    // �������� �� ������� ����������
    virtual bool empty() override
    {
        return _list->empty();
    }

    // ��������� ������� ���������� (���������� ���������)
    virtual int size() override
    {
        return _list->size();
    }

    // ������� ����������
    virtual void clear() override
    {
        _list->clear();
    }

    // �������� ������ ���������
    virtual Iterator* newIterator() override
    {
        return _list->newIterator();
    }
};