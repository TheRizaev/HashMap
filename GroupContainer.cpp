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
    : myContainer(container), index(startIndex), listIterator(new_it)
{
    currentList = nullptr;
    if (startIndex < container->arraySize) {
        currentList = container->Table[startIndex];
    }
}

GroupContainer::GroupContainerIterator::~GroupContainerIterator() {
    if (listIterator) {
        delete listIterator;
        listIterator = nullptr;
    }
}

void* GroupContainer::GroupContainerIterator::getElement(size_t& size)
{
    if (!currentList || !listIterator) {
        size = 0;
        return nullptr;
    }

    return listIterator->getElement(size);
}

bool GroupContainer::GroupContainerIterator::hasNext()
{
    // Если текущий список итератор имеет следующий элемент
    if (listIterator && listIterator->hasNext()) {
        return true;
    }

    // Ищем следующий непустой список
    for (size_t i = index + 1; i < myContainer->arraySize; i++) {
        if (myContainer->Table[i] && !myContainer->Table[i]->empty()) {
            return true;
        }
    }

    return false;
}

void GroupContainer::GroupContainerIterator::goToNext()
{
    if (!listIterator) {
        return;
    }

    // Если в текущем списке есть следующий элемент
    if (listIterator->hasNext()) {
        listIterator->goToNext();
        return;
    }

    // Переходим к следующему непустому списку
    delete listIterator;
    listIterator = nullptr;
    currentList = nullptr;

    for (size_t i = index + 1; i < myContainer->arraySize; i++) {
        if (myContainer->Table[i] && !myContainer->Table[i]->empty()) {
            index = i;
            currentList = myContainer->Table[i];
            listIterator = currentList->newIterator();
            break;
        }
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

void GroupContainer::remove(Iterator* iter)
{
    GroupContainerIterator* groupIter = dynamic_cast<GroupContainerIterator*>(iter);
    if (!groupIter || !groupIter->currentList || !groupIter->listIterator) {
        return;
    }

    // Получаем элемент перед удалением
    size_t elemSize;
    void* element = groupIter->listIterator->getElement(elemSize);

    if (element) {
        // Удаляем элемент из списка
        groupIter->currentList->remove(groupIter->listIterator);

        // Уменьшаем счетчик элементов в контейнере
        decreaseAmount();

        // Удаляем сам элемент (вызов виртуального метода)
        removeElement(element, elemSize);

        // ВАЖНО: Проверяем, не стал ли текущий список пустым
        if (groupIter->currentList->empty()) {
            // Если список стал пустым, переходим к следующему
            delete groupIter->listIterator;
            groupIter->listIterator = nullptr;
            groupIter->currentList = nullptr;

            // Ищем следующий непустой список
            for (size_t i = groupIter->index + 1; i < arraySize; i++) {
                if (Table[i] && !Table[i]->empty()) {
                    groupIter->index = i;
                    groupIter->currentList = Table[i];
                    groupIter->listIterator = groupIter->currentList->newIterator();
                    break;
                }
            }
        }
    }
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