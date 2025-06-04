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

    for (size_t i = 0; i < oldArraySize; i++) {
        if (oldTable[i]) {
            Iterator* iter = oldTable[i]->newIterator();
            if (!iter) {
                _memory.freeMem(oldTable[i]);
                continue;
            }
            while (true) {
                size_t elemSize;
                void* element = iter->getElement(elemSize);

                if (element) {
                    void* copy = _memory.allocMem(elemSize);
                    std::memcpy(copy, element, elemSize);

                    kv_pair* pair = static_cast<kv_pair*>(copy);
                    if (pair) {
                        size_t newIndex = hashFunc((char*)pair->key, pair->keySize);
                        if (!Table[newIndex]) {
                            Table[newIndex] = new List(_memory);
                        }
                        Table[newIndex]->push_front(copy, elemSize);
                        _memory.freeMem(copy);
                    }
                }

                if (!iter->hasNext()) break;
                iter->goToNext();
            }

            _memory.freeMem(iter);
        }
    }

    _memory.freeMem(oldTable);

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

    //for (size_t i = startIndex; i < myContainer->arraySize; i++) {
    //    if (myContainer->Table[i]) {
    //        currentList = myContainer->Table[i];
    //        listIterator = currentList->newIterator();
    //        index = i;
    //        break;
    //    }
    //}
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

    // Вызываем виртуальный метод для удаления из производного класса
    removeElement(element, elemSize);

    // Проверяем, есть ли следующий элемент в текущем списке
    bool currentListHasNext = gcIter->listIterator->hasNext();

    // Удаляем из списка
    gcIter->currentList->remove(gcIter->listIterator);
    decreaseAmount();

    // Если список стал пустым
    if (gcIter->currentList->size() == 0) {
        delete gcIter->currentList;
        Table[currentIndex] = nullptr;
        gcIter->currentList = nullptr;

        // Удаляем старый итератор списка
        if (gcIter->listIterator) {
            delete gcIter->listIterator;
            gcIter->listIterator = nullptr;
        }

        // Ищем следующий непустой список
        for (size_t i = currentIndex + 1; i < arraySize; i++) {
            if (Table[i] && Table[i]->size() > 0) {
                gcIter->currentList = Table[i];
                gcIter->listIterator = Table[i]->newIterator();
                gcIter->index = i;
                return;
            }
        }

        // Если не нашли следующий список, итератор указывает на конец
        gcIter->index = arraySize;
    }
    // Если список не пустой, но мы удалили последний элемент
    else if (!currentListHasNext) {
        // Перемещаемся к следующему списку
        if (gcIter->listIterator) {
            delete gcIter->listIterator;
            gcIter->listIterator = nullptr;
        }

        // Ищем следующий непустой список
        for (size_t i = currentIndex + 1; i < arraySize; i++) {
            if (Table[i] && Table[i]->size() > 0) {
                gcIter->currentList = Table[i];
                gcIter->listIterator = Table[i]->newIterator();
                gcIter->index = i;
                return;
            }
        }

        // Если не нашли, итератор на конец
        gcIter->currentList = nullptr;
        gcIter->index = arraySize;
    }
    // Если в текущем списке есть еще элементы, итератор уже указывает на следующий
}

void GroupContainer::clear() {
    for (size_t i = 0; i < arraySize; i++) {
        if (Table[i]) {
            Table[i]->clear();
            _memory.freeMem(Table[i]);
            Table[i] = nullptr;
        }
    }
    amountOfElements = 0;
}

