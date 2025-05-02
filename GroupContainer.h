#pragma once
#include "Container.h"
#include "List.h"
#include "MemoryManager.h"
#include <string>

class GroupContainer : public Container
{
protected:
    int amountOfElements;
    size_t arraySize;
    List** Table;

    double getLoadFactor();

    void increaseAmount();
    void decreaseAmount();

    virtual bool reHash();

    size_t hashFunc(char* key, size_t keySize);

    size_t getArraySize() const;
    List** getTable() const;

public:
    GroupContainer(MemoryManager& mem);

    virtual ~GroupContainer();

    bool empty() override;
    int size() override;
    size_t max_bytes() override;

    class GroupContainerIterator : public Iterator {
    protected:
        List* currentList;
        size_t index;
        Iterator* listIterator;
        GroupContainer* myContainer;

    public:
        GroupContainerIterator(GroupContainer* container, size_t startIndex, Iterator* new_it);

        ~GroupContainerIterator();

        void* getElement(size_t& size) override;

        bool hasNext() override;

        void goToNext() override;

        bool equals(Iterator* right) override;

        friend class GroupContainer;
    };

    virtual Iterator* newIterator() override;

    virtual void remove(Iterator* iter) override;

    void clear() override;

protected:
    virtual void removeElement(void* element, size_t elemSize) = 0;
};