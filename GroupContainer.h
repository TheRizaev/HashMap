#pragma once
#include "Container.h"
#include "List.h"
#include "MemoryManager.h"
#include <string>

// ������� ����� ��� ��������� ����������� (���������, �������)
class GroupContainer : public Container
{
protected:
    int amountOfElements;
    size_t arraySize;
    List** hashTable;

    // ��������� ������������ �������� ���-�������
    double getLoadFactor();

    // ��������� �������� ���������
    void increaseAmount();
    void decreaseAmount();

    // ��������������� - ���������� ������� ������� � ����������������� ���������
    virtual bool reHash() = 0;

    // ���-������� ��� ����� (���������� ������������� ������)
    size_t hashFunc(const std::string& key);

    // ������ � ���������� ������
    size_t getArraySize() const;
    List** getTable() const;

public:
    // �����������
    GroupContainer(MemoryManager& mem);

    // ����������
    virtual ~GroupContainer();

    // ������ �� �������� ������ Container
    bool empty() override;
    int size() override;
    size_t max_bytes() override;

    // ����� �������� ��� ���� ��������� �����������
    class GroupContainerIterator : public Iterator {
    protected:
        List* currentList;
        size_t index;
        Iterator* listIterator;
        GroupContainer* myContainer;

    public:
        // �����������
        GroupContainerIterator(GroupContainer* container, size_t startIndex = 0);

        // ����������
        ~GroupContainerIterator();

        // ��������� �������� ��������
        void* getElement(size_t& size) override;

        // �������� ������� ���������� ��������
        bool hasNext() override;

        // ������� � ���������� ��������
        void goToNext() override;

        // ��������� ����������
        bool equals(Iterator* right) override;

        friend class GroupContainer;
    };

    // �������� ������ ���������
    virtual Iterator* newIterator() override;

    // �������� �������� �� ���������
    virtual void remove(Iterator* iter) override;

    // ������� ����� ����������
    void clear() override;

protected:
    // ����� ��� ������������ ������ �������� (������ ���� ������������� � ��������)
    virtual void removeElement(void* element, size_t elemSize) = 0;

    // ����� ��� ������� ���� ��������� � ����� �������
    virtual void clearBucket(size_t bucketIndex) = 0;
};