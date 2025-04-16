#pragma once
#include "AbstractTable.h"
#include "GroupContainer.h"
#include "List.h"
#include "MemoryManager.h"
#include <string>

// ��������� ��� �������� ���� ����-��������
typedef struct KeyValuePair
{
    void* key;       // ��������� �� ����
    size_t keySize;  // ������ ����� � ������
    void* value;     // ��������� �� ��������
    size_t valueSize; // ������ �������� � ������
} KeyValuePair;

// ����� ������������� �������, ����������� �� AbstractTable � GroupContainer
class Table : public AbstractTable, public GroupContainer
{
public:
    // �����������, ����������� �������� ������
    Table(MemoryManager& mem);

    // ����������
    ~Table();

    // ������ ������ AbstractTable
    int insertByKey(void* key, size_t keySize, void* value, size_t valueSize) override;
    void* at(void* key, size_t keySize, size_t& valueSize) override;
    void removeByKey(void* key, size_t keySize) override;
    Iterator* findByKey(void* key, size_t keySize) override;

    // ����� �� Container, ���������������� ��� ������ �� ��������
    Iterator* find(void* elem, size_t size) override;

    // ����������� �������� �������, ����������� �� GroupContainerIterator
    class TableIterator : public GroupContainer::GroupContainerIterator
    {
    public:
        // �����������
        TableIterator(GroupContainer* container, size_t startIndex = 0);

        // �������������� getElement, ����� ���������� ������ ��������
        void* getElement(size_t& size) override;

        friend class Table;
    };

    // �������� ������ ��������� �������
    Iterator* newIterator() override;

protected:
    // ����� ��� �������� �������� (������������ ������ ����� � ��������)
    void removeElement(void* element, size_t elemSize) override;

    // ����� ��� ������� ���� ��������� � ����� �������
    void clearBucket(size_t bucketIndex) override;

    // ��������������� �������
    bool reHash() override;

private:
    // �������������� ����� � ������ ��� �����������
    std::string keyToString(void* key, size_t keySize);

    // ����� ���� ����-�������� �� �����
    Container::Iterator* findPairByKey(void* key, size_t keySize, size_t& bucketIndex);
};