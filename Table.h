#pragma once
#include "AbstractTable.h"
#include "List.h"
#include "GroupContainer.h"

// �����: ������������� ������� �� ������ ������������ ������
class Table : public AbstractTable
{
private:
    // ��������� ��� �������� ���� ����-��������
    struct KeyValuePair
    {
        void* key;
        size_t keySize;
        void* value;
        size_t valueSize;

        KeyValuePair(void* k, size_t kSize, void* v, size_t vSize)
            : key(k), keySize(kSize), value(v), valueSize(vSize) {}

        ~KeyValuePair()
        {
            // ������������� �������� ����������� �� ������� ����
        }
    };

    List _list;  // ����������� ������ ��� �������� ��� ����-��������

public:
    // ����� ��������� ��� �������
    class TableIterator : public Container::Iterator
    {
    private:
        Container::Iterator* _listIterator;  // �������� ������

    public:
        TableIterator(Container::Iterator* listIter) : _listIterator(listIter) {}

        ~TableIterator()
        {
            delete _listIterator;
        }

        // ���������� ������� ������� � ��� ������
        void* getElement(size_t& size) override;

        // ��������� ������� ���������� ��������
        bool hasNext() override;

        // ��������� � ���������� ��������
        void goToNext() override;

        // ��������� ��������� ����������
        bool equals(Container::Iterator* right) override;

        // �������� �������� ������
        Container::Iterator* getListIterator() { return _listIterator; }
    };

    // �����������
    Table(MemoryManager& mem);

    // ����������
    ~Table() override;

    // ���������� ������� Container
    int size() override;
    size_t max_bytes() override;
    Container::Iterator* find(void* elem, size_t size) override;
    Container::Iterator* newIterator() override;
    void remove(Container::Iterator* iter) override;
    void clear() override;
    bool empty() override;

    // ���������� ������� AbstractTable
    int insertByKey(void* key, size_t keySize, void* elem, size_t elemSize) override;
    void removeByKey(void* key, size_t keySize) override;
    Container::Iterator* findByKey(void* key, size_t keySize) override;
    void* at(void* key, size_t keySize, size_t& valueSize) override;

private:
    // ��������������� ������� ��� ��������� ������
    bool compareKeys(void* key1, size_t key1Size, void* key2, size_t key2Size);

    // ��������������� ������� ��� �������� ���� ����-��������
    KeyValuePair* createPair(void* key, size_t keySize, void* value, size_t valueSize);
};