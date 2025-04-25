#pragma once
#include "AbstractTable.h"
#include "GroupContainer.h"
#include <string>

typedef struct KeyValuePair
{
    void* key;
    size_t keySize;
    void* value;
    size_t valueSize;
} kv_pair;

class Table : public AbstractTable
{
private:
    static const double MAX_LOAD_FACTOR;

protected:
    char* keyToCharArray(void* key, size_t keySize);

    bool reHash() override;

    void removeElement(void* element, size_t elemSize) override;

    void clearBucket(size_t bucketIndex) override;

public:
    Table(MemoryManager& mem);

    ~Table();

    int insertByKey(void* key, size_t keySize, void* elem, size_t elemSize) override;
    void removeByKey(void* key, size_t keySize) override;
    Iterator* findByKey(void* key, size_t keySize) override;
    void* at(void* key, size_t keySize, size_t& valueSize) override;

    Iterator* find(void* elem, size_t size) override;

    class TableIterator : public GroupContainer::GroupContainerIterator {
    public:
        TableIterator(Table* table, size_t startIndex = 0);
        void* getValue(size_t& size);
        void* getKey(size_t& size);
    };
};