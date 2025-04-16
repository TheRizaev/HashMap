#include "GroupContainer.h"
#include <string>

// �����������
GroupContainer::GroupContainer(MemoryManager& mem) : Container(mem) {
    amountOfElements = 0;
    arraySize = 1000;
    hashTable = (List**)_memory.allocMem(sizeof(List*) * arraySize);
    for (size_t i = 0; i < arraySize; i++) {
        hashTable[i] = nullptr;
    }
}

// ����������
GroupContainer::~GroupContainer() {
    if (hashTable) {
        _memory.freeMem(hashTable);
        hashTable = nullptr;
    }
}

// ��������� ������������ �������� ���-�������
double GroupContainer::getLoadFactor() {
    if (arraySize == 0) return 0.0;
    return static_cast<double>(amountOfElements) / arraySize;
}

// ��������� �������� ���������
void GroupContainer::increaseAmount() {
    amountOfElements++;
}

void GroupContainer::decreaseAmount() {
    if (amountOfElements > 0) amountOfElements--;
}

// ���-������� ��� ����� (���������� ������������� ������)
size_t GroupContainer::hashFunc(const std::string& key) {
    size_t hash = 0;
    for (char c : key) {
        hash = hash * 31 + c;
    }
    return hash;
}

// ������ � ���������� ������
size_t GroupContainer::getArraySize() const {
    return arraySize;
}

List** GroupContainer::getTable() const {
    return hashTable;
}

// ������ �� �������� ������ Container
bool GroupContainer::empty() {
    return amountOfElements == 0;
}

int GroupContainer::size() {
    return amountOfElements;
}

size_t GroupContainer::max_bytes() {
    return arraySize * sizeof(List*);
}

// ����������� ���������
GroupContainer::GroupContainerIterator::GroupContainerIterator(GroupContainer* container, size_t startIndex)
    : myContainer(container), index(startIndex), currentList(nullptr), listIterator(nullptr)
{
    if (!myContainer || !myContainer->hashTable)
        return;

    // ������� ������ �������� ������
    for (size_t i = startIndex; i < myContainer->arraySize; i++) {
        if (myContainer->hashTable[i]) {
            currentList = myContainer->hashTable[i];
            listIterator = currentList->newIterator();
            index = i;
            break;
        }
    }
}

// ���������� ���������
GroupContainer::GroupContainerIterator::~GroupContainerIterator() {
    if (listIterator) {
        delete listIterator;
        listIterator = nullptr;
    }
}

// ��������� �������� ��������
void* GroupContainer::GroupContainerIterator::getElement(size_t& size) {
    if (!currentList || !listIterator) {
        size = 0;
        return nullptr;
    }
    return listIterator->getElement(size);
}

// �������� ������� ���������� ��������
bool GroupContainer::GroupContainerIterator::hasNext() {
    if (!myContainer || !myContainer->hashTable)
        return false;

    // ���� ������� ������ ���� � � ��� ���� ��������� �������
    if (currentList && listIterator && listIterator->hasNext())
        return true;

    // ���� ��������� �������� ������
    for (size_t i = index + 1; i < myContainer->arraySize; i++) {
        if (myContainer->hashTable[i])
            return true;
    }

    return false;
}

// ������� � ���������� ��������
void GroupContainer::GroupContainerIterator::goToNext() {
    if (!myContainer || !myContainer->hashTable)
        return;

    // ���� � ������� ������ ���� ��������� �������
    if (currentList && listIterator && listIterator->hasNext()) {
        listIterator->goToNext();
        return;
    }

    // ����������� ������� ��������
    if (listIterator) {
        delete listIterator;
        listIterator = nullptr;
    }

    // ���� ��������� �������� ������
    for (size_t i = index + 1; i < myContainer->arraySize; i++) {
        if (myContainer->hashTable[i]) {
            currentList = myContainer->hashTable[i];
            listIterator = currentList->newIterator();
            index = i;
            return;
        }
    }

    // ���� �� ����� ��������� ������
    currentList = nullptr;
}

// ��������� ����������
bool GroupContainer::GroupContainerIterator::equals(Iterator* right) {
    if (!right)
        return false;

    GroupContainerIterator* rightIter = dynamic_cast<GroupContainerIterator*>(right);
    if (!rightIter)
        return false;

    // ���������� ��������� �� ��������� � ������
    if (myContainer != rightIter->myContainer || index != rightIter->index)
        return false;

    // ���� ��� ��������� ��������� �� ����� (��� �������� ������)
    if (!currentList && !rightIter->currentList)
        return true;

    // ���� ���� �� ���������� ��������� �� �����
    if (!currentList || !rightIter->currentList)
        return false;

    // ���������� ��������� ������
    if (!listIterator || !rightIter->listIterator)
        return false;

    return listIterator->equals(rightIter->listIterator);
}

// �������� ������ ���������
Iterator* GroupContainer::newIterator() {
    if (empty())
        return nullptr;
    return new GroupContainerIterator(this);
}

// �������� �������� �� ���������
void GroupContainer::remove(Iterator* iter) {
    if (!iter) return;

    GroupContainerIterator* gcIter = dynamic_cast<GroupContainerIterator*>(iter);
    if (!gcIter || !gcIter->currentList || !gcIter->listIterator) return;

    // ������� ������� �� ������
    size_t elemSize;
    void* element = gcIter->listIterator->getElement(elemSize);
    if (element) {
        // ������� ������ �������� (���������� ��� ������� ����������)
        removeElement(element, elemSize);

        // ������� ������� �� ������
        gcIter->currentList->remove(gcIter->listIterator);

        // ��������� ������� ���������
        decreaseAmount();

        // ���� ������ ���� ������, ������� ���
        if (gcIter->currentList->empty()) {
            delete gcIter->currentList;
            hashTable[gcIter->index] = nullptr;
            gcIter->currentList = nullptr;
        }

        // �������� ��������� �� �������� ������
        gcIter->listIterator = nullptr;

        // ��������� � ���������� ��������
        gcIter->goToNext();
    }
}

// ������� ����� ����������
void GroupContainer::clear() {
    for (size_t i = 0; i < arraySize; i++) {
        if (hashTable[i]) {
            clearBucket(i);
            delete hashTable[i];
            hashTable[i] = nullptr;
        }
    }
    amountOfElements = 0;
}