#pragma once
#include "AbstractTable.h"
#include "List.h"
#include "MemoryManager.h"

// ��������� ��� �������� ���� ����-��������
typedef struct KeyValuePair
{
    void* key;       // ��������� �� ����
    size_t keySize;  // ������ ����� � ������
    void* value;     // ��������� �� ��������
    size_t valueSize; // ������ �������� � ������
} KeyValuePair;

// ����� ������������� �������, ����������� �� AbstractTable
class Table : public AbstractTable
{
private:
    List* _list;     // ���������� ������ ��� �������� ���������

public:
    // �����������, ����������� �������� ������
    Table(MemoryManager& mem);

    // ����������
    ~Table();

    // ������ �������� ������ Container
    bool empty() override;
    int size() override;
    void clear() override;
    Iterator* newIterator() override;

    // ������ ������ AbstractTable
    int insertByKey(void* key, size_t keySize, void* value, size_t valueSize) override;
    void* at(void* key, size_t keySize, size_t& valueSize) override;
    void removeByKey(void* key, size_t keySize) override;

    // �����-�������� ��� ������ ������������� �������
    class TableIterator : public Iterator
    {
    private:
        Iterator* _listIterator; // ���������� �������� ��� ������ ������

    public:
        // �����������, ����������� �������� ������
        TableIterator(Iterator* listIterator);

        // ����������
        ~TableIterator();

        // ������ �������� ������ Iterator
        void* getElement(size_t& size) override;
        bool hasNext() override;
        void goToNext() override;
        bool equals(Iterator* right) override;
    };

private:
    // ����� ���� ����-�������� �� �����
    // ���������� �������� �� ��������� ������� ��� nullptr, ���� ���� �� ������
    Container::Iterator* findPairByKey(void* key, size_t keySize);
};