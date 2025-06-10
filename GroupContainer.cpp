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

bool GroupContainer::reHash() {
    size_t oldArraySize = arraySize;
    List** oldTable = Table;

    size_t newArraySize = arraySize * 2;
    List** newTable = (List**)_memory.allocMem(sizeof(List*) * newArraySize);
    if (!newTable) return false;

    for (size_t i = 0; i < newArraySize; i++) {
        newTable[i] = nullptr;
    }

    Table = newTable;
    size_t tempArraySize = arraySize;
    arraySize = newArraySize;

    // ВАЖНО: Сохраняем текущее количество элементов
    int savedAmountOfElements = amountOfElements;
    amountOfElements = 0;  // Временно обнуляем для корректного подсчета

    for (size_t i = 0; i < oldArraySize; i++) {
        if (oldTable[i]) {
            Iterator* iter = oldTable[i]->newIterator();
            if (!iter) {
                // Очищаем список перед удалением
                oldTable[i]->clear();
                delete oldTable[i];
                continue;
            }

            while (true) {
                size_t elemSize;
                void* element = iter->getElement(elemSize);

                if (element) {
                    kv_pair* pair = static_cast<kv_pair*>(element);
                    if (pair) {
                        size_t newIndex = hashFunc((char*)pair->key, pair->keySize);
                        if (!Table[newIndex]) {
                            Table[newIndex] = new List(_memory);
                        }
                        // Передаем элемент напрямую, push_front сделает свою копию
                        Table[newIndex]->push_front(element, elemSize);
                        amountOfElements++;  // Увеличиваем счетчик при добавлении
                    }
                }

                if (!iter->hasNext()) break;
                iter->goToNext();
            }

            delete iter;

            // Очищаем старый список
            oldTable[i]->clear();
            delete oldTable[i];
        }
    }

    _memory.freeMem(oldTable);

    // Проверяем, что количество элементов не изменилось
    if (amountOfElements != savedAmountOfElements) {
        // Это не должно происходить, но если происходит - восстанавливаем
        amountOfElements = savedAmountOfElements;
    }

    return true;
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

void GroupContainer::remove(Iterator* iter) {
    if (!iter) return;

    GroupContainerIterator* gcIter = dynamic_cast<GroupContainerIterator*>(iter);
    if (!gcIter || !gcIter->currentList || !gcIter->listIterator) return;

    size_t elemSize;
    void* element = gcIter->listIterator->getElement(elemSize);
    if (!element) return;

    size_t currentIndex = gcIter->index;

    // Save the current state before removal
    bool hasNext = gcIter->listIterator->hasNext();

    // Remove the element from the container (custom implementation in derived class)
    removeElement(element, elemSize);

    // Remove from the list and update container count
    gcIter->currentList->remove(gcIter->listIterator);
    decreaseAmount();

    // Check if the list is now empty
    if (gcIter->currentList->size() == 0) {
        delete gcIter->currentList;
        Table[currentIndex] = nullptr;
        gcIter->currentList = nullptr;

        // Find the next non-empty list
        bool found = false;
        for (size_t i = currentIndex + 1; i < arraySize; i++) {
            if (Table[i]) {
                gcIter->currentList = Table[i];
                if (gcIter->listIterator) {
                    delete gcIter->listIterator;
                }
                gcIter->listIterator = Table[i]->newIterator();
                gcIter->index = i;
                found = true;
                break;
            }
        }

        if (!found) {
            if (gcIter->listIterator) {
                delete gcIter->listIterator;
                gcIter->listIterator = nullptr;
            }
        }
    }
    // If the list is not empty but the current element was the last one in the list
    else if (!hasNext) {
        // Reset iterator to the beginning of the current list
        if (gcIter->listIterator) {
            delete gcIter->listIterator;
        }
        gcIter->listIterator = gcIter->currentList->newIterator();
    }
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