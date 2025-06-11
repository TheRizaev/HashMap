#include "GroupContainer.h"
#include "Table.h"
#include <string>

GroupContainer::GroupContainer(MemoryManager& mem) : Container(mem) {
    amountOfElements = 0;
    arraySize = 1000;
    Table = (List**)_memory.allocMem(sizeof(List*) * arraySize);
    for (size_t i = 0; i < arraySize; i++) {
        Table[i] = nullptr;
    }
}

GroupContainer::~GroupContainer() {
    if (Table) {
        _memory.freeMem(Table);
        Table = nullptr;
    }
}

double GroupContainer::getLoadFactor() {
    if (arraySize == 0) return 0.0;
    return static_cast<double>(amountOfElements) / arraySize;
}

void GroupContainer::increaseAmount() {
    amountOfElements++;
}

void GroupContainer::decreaseAmount() {
    if (amountOfElements > 0) {
        amountOfElements--;
    }
}

size_t GroupContainer::hashFunc(char* key, size_t keySize) {
    size_t hash = 0;
    for (size_t i = 0; i < keySize; i++)
        hash = hash * 255 + key[i];
    return hash % arraySize;
}

char* GroupContainer::ElemToCharArray(void* key, size_t keySize) {
    char* ptr = (char*)_memory.allocMem(keySize);
    memcpy(ptr, key, keySize);
    return ptr;
}

size_t GroupContainer::getArraySize() const {
    return arraySize;
}

List** GroupContainer::getTable() const {
    return Table;
}

bool GroupContainer::empty() {
    return amountOfElements == 0;
}

int GroupContainer::size() {
    return amountOfElements;
}

size_t GroupContainer::max_bytes() {
    return arraySize * sizeof(List*);
}

GroupContainer::GroupContainerIterator::GroupContainerIterator(GroupContainer* container, size_t startIndex, Iterator* new_it)
    : myContainer(container), index(startIndex), currentList(nullptr), listIterator(nullptr)
{
    if (!myContainer || !myContainer->Table)
        return;

    currentList = myContainer->Table[startIndex];
    listIterator = new_it;
    index = startIndex;
}

GroupContainer::GroupContainerIterator::~GroupContainerIterator() {
    if (listIterator) {
        delete listIterator;
        listIterator = nullptr;
    }
}

void* GroupContainer::GroupContainerIterator::getElement(size_t& size) {
    if (!currentList || !listIterator) {
        size = 0;
        return nullptr;
    }
    return listIterator->getElement(size);
}

bool GroupContainer::GroupContainerIterator::hasNext() {
    if (!myContainer || !myContainer->Table)
        return false;

    if (currentList && listIterator && listIterator->hasNext())
        return true;

    for (size_t i = index + 1; i < myContainer->arraySize; i++) {
        if (myContainer->Table[i])
            return true;
    }

    return false;
}

void GroupContainer::GroupContainerIterator::goToNext() {
    if (!myContainer || !myContainer->Table)
        return;

    if (currentList && listIterator && listIterator->hasNext()) {
        listIterator->goToNext();
        return;
    }

    if (listIterator) {
        delete listIterator;
        listIterator = nullptr;
    }

    currentList = nullptr;

    size_t i = index + 1;
    while (i < myContainer->arraySize) {
        if (myContainer->Table[i]) {
            currentList = myContainer->Table[i];

            if (currentList->size() > 0) {
                listIterator = currentList->newIterator();
                if (listIterator) {
                    index = i;
                    return;
                }
            }
        }
        i++;
    }
}

bool GroupContainer::GroupContainerIterator::equals(Iterator* right) {
    if (!right)
        return false;

    GroupContainerIterator* rightIter = dynamic_cast<GroupContainerIterator*>(right);
    if (!rightIter)
        return false;

    if (myContainer != rightIter->myContainer || index != rightIter->index)
        return false;

    if (!currentList && !rightIter->currentList)
        return true;

    if (!currentList || !rightIter->currentList)
        return false;

    if (!listIterator || !rightIter->listIterator)
        return false;

    return listIterator->equals(rightIter->listIterator);
}

Container::Iterator* GroupContainer::newIterator() {
    if (empty())
        return nullptr;
    size_t i = 0;
    for (; i < arraySize; i++)
        if (Table[i])
            break;
    if (i == arraySize)
        return nullptr;
    return new GroupContainerIterator(this, i, Table[i]->newIterator());
}

void GroupContainer::clear() {
    for (size_t i = 0; i < arraySize; i++) {
        if (Table[i]) {
            Table[i]->clear();
            delete Table[i];  // Изменено с _memory.freeMem на delete
            Table[i] = nullptr;
        }
    }
    amountOfElements = 0;
}