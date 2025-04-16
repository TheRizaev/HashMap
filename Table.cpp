#include "Table.h"
#include <cstring> // ��� memcmp

// ����������� �������
Table::Table(MemoryManager& mem)
    : AbstractTable(mem), GroupContainer(mem)
{
    // ��� ����������� �������� ��� ��������� � ������������� ������� �������
}

// ���������� �������
Table::~Table()
{
    clear(); // ������� �������
    // ������������ ������ ��� ������� ����������� � ����������� GroupContainer
}

// �������������� ����� � ������ ��� �����������
std::string Table::keyToString(void* key, size_t keySize)
{
    if (!key || keySize == 0)
        return "";

    // ����������� ���� � ������
    char* keyPtr = static_cast<char*>(key);
    // ����������� � std::string (�� ������� ������� ������ � �����, ���� �� ����)
    return std::string(keyPtr, keySize - 1);
}

// ��������������� ������� (���������� �������)
bool Table::reHash()
{
    size_t oldSize = arraySize;
    size_t newSize = arraySize * 2;

    // ��������� ������ �������
    List** oldTable = hashTable;

    // ������� ����� �������
    hashTable = (List**)_memory.allocMem(sizeof(List*) * newSize);
    if (!hashTable) {
        hashTable = oldTable;  // ��������������� ������ �������
        return false;
    }

    // �������������� ����� �������
    for (size_t i = 0; i < newSize; i++) {
        hashTable[i] = nullptr;
    }

    // ��������� ������ �������
    arraySize = newSize;

    // ���������� ������� ��������� (����� ������ ������� ��� ���������������)
    amountOfElements = 0;

    // ������������ �������� �� ������ �������
    for (size_t i = 0; i < oldSize; i++) {
        if (oldTable[i]) {
            Container::Iterator* iter = oldTable[i]->newIterator();
            if (iter) {
                while (true) {
                    size_t elemSize;
                    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(elemSize));

                    if (pair) {
                        // ��������� ����� ��� � ��������� �������
                        std::string keyStr = keyToString(pair->key, pair->keySize);
                        size_t hash = hashFunc(keyStr) % arraySize;

                        // ���� � ������ ������� ��� ��� ������, ������� ���
                        if (!hashTable[hash]) {
                            hashTable[hash] = new List(_memory);
                        }

                        // ��������� ���� � ������
                        if (hashTable[hash]->push_front(pair, sizeof(KeyValuePair)) == 0) {
                            increaseAmount();
                        }
                    }

                    if (!iter->hasNext())
                        break;

                    iter->goToNext();
                }

                delete iter;
            }

            // ������� ������ ������
            delete oldTable[i];
        }
    }

    // ����������� ������, ���������� ��� ������ �������
    _memory.freeMem(oldTable);

    return true;
}

// ������� �������� �� �����
int Table::insertByKey(void* key, size_t keySize, void* value, size_t valueSize)
{
    // ���������, ���������� �� ��� ������� � ����� ������
    size_t bucketIndex;
    if (findPairByKey(key, keySize, bucketIndex) != nullptr)
    {
        // ���� ��� ����������, ���������� ������
        return 1;
    }

    // ��������� ��� �����
    std::string keyStr = keyToString(key, keySize);
    size_t hash = hashFunc(keyStr) % arraySize;

    // ���� �������� ��������� �����, ����������� ������ �������
    if (getLoadFactor() > 0.75) {
        reHash();
        // ������������� ��� ����� ��������� ������� �������
        hash = hashFunc(keyStr) % arraySize;
    }

    // ���� � ������ ������� ��� ��� ������, ������� ���
    if (!hashTable[hash]) {
        hashTable[hash] = new List(_memory);
    }

    // ������� ���� ����-��������
    KeyValuePair pair;

    // �������� ������ ��� ����� � ��������
    pair.key = _memory.allocMem(keySize);
    if (!pair.key) {
        return 1; // ������ ��������� ������
    }

    pair.keySize = keySize;

    pair.value = _memory.allocMem(valueSize);
    if (!pair.value) {
        _memory.freeMem(pair.key);
        return 1;
    }

    pair.valueSize = valueSize;

    // �������� ������ ����� � ��������
    memcpy(pair.key, key, keySize);
    memcpy(pair.value, value, valueSize);

    // ��������� ���� � ������
    int result = hashTable[hash]->push_front(&pair, sizeof(KeyValuePair));

    // ��������� ��������� ��������
    if (result != 0) {
        // ���� ��������� ������, ����������� ���������� ������
        _memory.freeMem(pair.key);
        _memory.freeMem(pair.value);
    }
    else {
        // ����������� ������� ���������
        increaseAmount();
    }

    return result;
}

// ��������� �������� �� �����
void* Table::at(void* key, size_t keySize, size_t& valueSize)
{
    size_t bucketIndex;
    Container::Iterator* iter = findPairByKey(key, keySize, bucketIndex);
    if (!iter)
    {
        // ���� �� ������
        valueSize = 0;
        return nullptr;
    }

    // �������� ���� ����-��������
    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

    // ��������� ������ ��������
    valueSize = pair->valueSize;

    // ��������� ��������� �� ��������
    void* result = pair->value;

    // ������� ��������
    delete iter;

    // ���������� ��������� �� ��������
    return result;
}

// �������� �������� �� �����
void Table::removeByKey(void* key, size_t keySize)
{
    size_t bucketIndex;
    Container::Iterator* iter = findPairByKey(key, keySize, bucketIndex);
    if (!iter)
    {
        // ���� �� ������
        return;
    }

    // �������� ���� ����-�������� ��� ������������ ������
    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

    // ����������� ������ ��� ����� � ��������
    _memory.freeMem(pair->key);
    _memory.freeMem(pair->value);

    // ������� ������� �� ������
    hashTable[bucketIndex]->remove(iter);

    // ��������� ������� ���������
    decreaseAmount();

    // ���� ������ ���� ������, ������� ���
    if (hashTable[bucketIndex]->empty()) {
        delete hashTable[bucketIndex];
        hashTable[bucketIndex] = nullptr;
    }

    // �������� ��� ������ � hashTable[bucketIndex]->remove(iter)
}

// ����� ���� ����-�������� �� �����
Container::Iterator* Table::findPairByKey(void* key, size_t keySize, size_t& bucketIndex)
{
    // ��������� ��� �����
    std::string keyStr = keyToString(key, keySize);
    size_t hash = hashFunc(keyStr) % arraySize;
    bucketIndex = hash;

    // ���������, ���� �� ������ � ������ �������
    if (!hashTable[hash])
    {
        // ������ ����
        return nullptr;
    }

    // ������� �������� ��� ������
    Container::Iterator* iter = hashTable[hash]->newIterator();
    if (!iter)
    {
        // �� ������� ������� ��������
        return nullptr;
    }

    // ������� ������ � ���� ���� � ��������� ������
    while (true)
    {
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

        if (!pair)
        {
            // ����������� ������
            delete iter;
            return nullptr;
        }

        // ���������� �����
        if (pair->keySize == keySize && memcmp(pair->key, key, keySize) == 0)
        {
            // ���� ������
            return iter;
        }

        // ��������� � ���������� ��������
        if (iter->hasNext())
        {
            iter->goToNext();
        }
        else
        {
            // ��������� ����� ������, ���� �� ������
            delete iter;
            return nullptr;
        }
    }
}

// ���������� ������ findByKey �� AbstractTable
Container::Iterator* Table::findByKey(void* key, size_t keySize)
{
    size_t bucketIndex;
    Container::Iterator* iter = findPairByKey(key, keySize, bucketIndex);
    if (!iter)
    {
        return nullptr;
    }

    // ������� �������� �������, ������� ���������� � ���������� ��������
    TableIterator* tableIter = new TableIterator(this, bucketIndex);

    // ������� �������� ��������, ��� ��� �� ������� �����
    delete iter;

    return tableIter;
}

// ����� �������� �� ��������
Container::Iterator* Table::find(void* elem, size_t size)
{
    // ���������� ��� ������ � ���� ��������
    for (size_t i = 0; i < arraySize; i++) {
        if (hashTable[i]) {
            Container::Iterator* iter = hashTable[i]->newIterator();
            if (iter) {
                while (true) {
                    size_t pairSize;
                    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

                    if (pair && pair->valueSize == size && memcmp(pair->value, elem, size) == 0) {
                        // �������� �������
                        // ������� �������� �������, ������� ���������� � ���������� ��������
                        TableIterator* tableIter = new TableIterator(this, i);

                        // ������� �������� ��������, ��� ��� �� ������� �����
                        delete iter;

                        return tableIter;
                    }

                    if (!iter->hasNext())
                        break;

                    iter->goToNext();
                }

                delete iter;
            }
        }
    }

    return nullptr;
}

// �������� ������ ���������
Container::Iterator* Table::newIterator()
{
    if (empty())
        return nullptr;

    return new TableIterator(this);
}

// ����� ��� �������� �������� (������������ ������ ����� � ��������)
void Table::removeElement(void* element, size_t elemSize)
{
    if (!element || elemSize != sizeof(KeyValuePair))
        return;

    KeyValuePair* pair = static_cast<KeyValuePair*>(element);
    _memory.freeMem(pair->key);
    _memory.freeMem(pair->value);
}

// ����� ��� ������� ���� ��������� � ����� �������
void Table::clearBucket(size_t bucketIndex)
{
    if (bucketIndex >= arraySize || !hashTable[bucketIndex])
        return;

    Container::Iterator* iter = hashTable[bucketIndex]->newIterator();
    if (iter) {
        while (true) {
            size_t pairSize;
            KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

            if (pair) {
                // ����������� ������ ��� ����� � ��������
                _memory.freeMem(pair->key);
                _memory.freeMem(pair->value);
            }

            if (!iter->hasNext())
                break;

            iter->goToNext();
        }
        delete iter;
    }
}

// ���������� ��������� �������

// ����������� ��������� �������
Table::TableIterator::TableIterator(GroupContainer* container, size_t startIndex)
    : GroupContainerIterator(container, startIndex)
{
}

// �������������� getElement, ����� ���������� ������ ��������
void* Table::TableIterator::getElement(size_t& size)
{
    if (!currentList || !listIterator) {
        size = 0;
        return nullptr;
    }

    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(listIterator->getElement(pairSize));
    if (!pair) {
        size = 0;
        return nullptr;
    }

    // ���������� �������� � ��� ������
    size = pair->valueSize;
    return pair->value;
}