#pragma once
#include "Container.h"
#include "List.h"
#include "MemoryManager.h"
#include <string>

// ������� ����� ��� ��������� ����������� (���������, �������)
class GroupContainer : public Container
{
protected:
    int amountOfElements;
    size_t arraySize;
    List** hashTable;

    // ��������� ������������ �������� ���-�������
    double getLoadFactor() {
        if (arraySize == 0) return 0.0;
        return static_cast<double>(amountOfElements) / arraySize;
    }

    // ��������� �������� ���������
    void increaseAmount() { amountOfElements++; }
    void decreaseAmount() { if (amountOfElements > 0) amountOfElements--; }

    // ��������������� - ���������� ������� ������� � ����������������� ���������
    virtual bool reHash() = 0;

    // ���-������� ��� ����� (���������� ������������� ������)
    size_t hashFunc(const std::string& key) {
        size_t hash = 0;
        for (char c : key) {
            hash = hash * 31 + c;
        }
        return hash;
    }

    // ������ � ���������� ������
    size_t getArraySize() const { return arraySize; }
    List** getTable() const { return hashTable; }

public:
    // �����������
    GroupContainer(MemoryManager& mem) : Container(mem) {
        amountOfElements = 0;
        arraySize = 1000;
        hashTable = (List**)_memory.allocMem(sizeof(List*) * arraySize);
        for (size_t i = 0; i < arraySize; i++) {
            hashTable[i] = nullptr;
        }
    }

    // ����������
    virtual ~GroupContainer() {
        if (hashTable) {
            _memory.freeMem(hashTable);
            hashTable = nullptr;
        }
    }

    // ������ �� �������� ������ Container
    bool empty() override { return amountOfElements == 0; }
    int size() override { return amountOfElements; }
    size_t max_bytes() override { return arraySize * sizeof(List*); }

    // ����� �������� ��� ���� ��������� �����������
    class GroupContainerIterator : public Iterator {
    protected:
        List* currentList;
        size_t index;
        Iterator* listIterator;
        GroupContainer* myContainer;

    public:
        // �����������
        GroupContainerIterator(GroupContainer* container, size_t startIndex = 0)
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

        // ����������
        ~GroupContainerIterator() {
            if (listIterator) {
                delete listIterator;
                listIterator = nullptr;
            }
        }

        // ��������� �������� ��������
        void* getElement(size_t& size) override {
            if (!currentList || !listIterator) {
                size = 0;
                return nullptr;
            }
            return listIterator->getElement(size);
        }

        // �������� ������� ���������� ��������
        bool hasNext() override {
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
        void goToNext() override {
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
        bool equals(Iterator* right) override {
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

        friend class GroupContainer;
    };

    // �������� ������ ���������
    virtual Iterator* newIterator() override {
        if (empty())
            return nullptr;
        return new GroupContainerIterator(this);
    }

    // �������� �������� �� ���������
    virtual void remove(Iterator* iter) override {
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
    void clear() override {
        for (size_t i = 0; i < arraySize; i++) {
            if (hashTable[i]) {
                clearBucket(i);
                delete hashTable[i];
                hashTable[i] = nullptr;
            }
        }
        amountOfElements = 0;
    }

protected:
    // ����� ��� ������������ ������ �������� (������ ���� ������������� � ��������)
    virtual void removeElement(void* element, size_t elemSize) = 0;

    // ����� ��� ������� ���� ��������� � ����� �������
    virtual void clearBucket(size_t bucketIndex) = 0;
};