#include "Table.h"
#include <cstring>

const double Table::MAX_LOAD_FACTOR = 0.75;

Table::Table(MemoryManager& mem) : AbstractTable(mem) {
    // Инициализация уже происходит в конструкторе GroupContainer
}

Table::~Table() {
    // Очистка выполняется в базовом классе
    clear();
}

// Преобразовать ключ в char* для хеширования
char* Table::keyToCharArray(void* key, size_t keySize) {
    char* keyChars = (char*)_memory.allocMem(keySize + 1);
    if (!keyChars) return nullptr;

    memcpy(keyChars, key, keySize);
    keyChars[keySize] = '\0'; // Добавление нулевого символа для безопасности

    return keyChars;
}

bool Table::reHash() {
    size_t oldSize = getArraySize();
    List** oldTable = getTable();

    size_t newSize = oldSize * 2;
    List** newTable = (List**)_memory.allocMem(sizeof(List*) * newSize);

    if (!newTable) {
        return false;
    }

    for (size_t i = 0; i < newSize; i++) {
        newTable[i] = nullptr;
    }

    List** tempTable = hashTable;
    hashTable = newTable;
    size_t tempSize = arraySize;
    arraySize = newSize;

    int oldCount = amountOfElements;
    amountOfElements = 0;

    for (size_t i = 0; i < oldSize; i++) {
        if (oldTable[i]) {
            Container::Iterator* iter = oldTable[i]->newIterator();

            if (iter) {
                do {
                    size_t elemSize;
                    kv_pair* pair = (kv_pair*)iter->getElement(elemSize);

                    if (pair) {
                        insertByKey(pair->key, pair->keySize, pair->value, pair->valueSize);

                        _memory.freeMem(pair->key);
                        _memory.freeMem(pair->value);
                    }

                    if (!iter->hasNext()) {
                        break;
                    }

                    iter->goToNext();
                } while (true);

                delete iter;
            }

            delete oldTable[i];
        }
    }

    _memory.freeMem(oldTable);

    if (amountOfElements != oldCount) {
        return false;
    }

    return true;
}

void Table::removeElement(void* element, size_t elemSize) {
    if (!element) return;

    kv_pair* pair = (kv_pair*)element;

    if (pair->key) {
        _memory.freeMem(pair->key);
    }

    if (pair->value) {
        _memory.freeMem(pair->value);
    }
}

void Table::clearBucket(size_t bucketIndex) {
    if (!hashTable || bucketIndex >= arraySize || !hashTable[bucketIndex]) {
        return;
    }

    List* bucket = hashTable[bucketIndex];
    Container::Iterator* iter = bucket->newIterator();

    if (iter) {
        do {
            size_t elemSize;
            kv_pair* pair = (kv_pair*)iter->getElement(elemSize);

            if (pair) {
                _memory.freeMem(pair->key);
                _memory.freeMem(pair->value);
            }

            if (!iter->hasNext()) {
                break;
            }

            iter->goToNext();
        } while (true);

        delete iter;
    }
}

int Table::insertByKey(void* key, size_t keySize, void* elem, size_t elemSize) {
    if (findByKey(key, keySize) != nullptr) {
        return 1;
    }

    if (getLoadFactor() >= MAX_LOAD_FACTOR) {
        reHash();
    }

    char* keyChars = keyToCharArray(key, keySize);
    if (!keyChars) return 1;

    size_t hash = hashFunc(keyChars, keySize) % arraySize;
    _memory.freeMem(keyChars);

    if (!hashTable[hash]) {
        hashTable[hash] = new List(_memory);
    }

    kv_pair* pair = (kv_pair*)_memory.allocMem(sizeof(kv_pair));

    pair->key = _memory.allocMem(keySize);
    memcpy(pair->key, key, keySize);
    pair->keySize = keySize;

    pair->value = _memory.allocMem(elemSize);
    memcpy(pair->value, elem, elemSize);
    pair->valueSize = elemSize;

    int result = hashTable[hash]->push_front(pair, sizeof(kv_pair));

    if (result == 0) {
        increaseAmount();
    }
    else {
        _memory.freeMem(pair->key);
        _memory.freeMem(pair->value);
        _memory.freeMem(pair);
    }

    return result;
}

void Table::removeByKey(void* key, size_t keySize) {
    Iterator* iter = findByKey(key, keySize);

    if (iter != nullptr) {
        remove(iter);
        delete iter;
    }
}

Container::Iterator* Table::findByKey(void* key, size_t keySize) {
    if (!key || keySize == 0) {
        return nullptr;
    }

    char* keyChars = keyToCharArray(key, keySize);
    if (!keyChars) return nullptr;

    size_t hash = hashFunc(keyChars, keySize) % arraySize;
    _memory.freeMem(keyChars);

    if (!hashTable[hash]) {
        return nullptr;
    }

    Container::Iterator* iter = hashTable[hash]->newIterator();

    if (!iter) {
        return nullptr;
    }

    do {
        size_t elemSize;
        kv_pair* pair = (kv_pair*)iter->getElement(elemSize);

        if (pair && pair->keySize == keySize && memcmp(pair->key, key, keySize) == 0) {
            TableIterator* tableIter = new TableIterator(this, hash);
            delete iter;
            return tableIter;
        }

        if (!iter->hasNext()) {
            break;
        }

        iter->goToNext();
    } while (true);

    delete iter;
    return nullptr;
}

void* Table::at(void* key, size_t keySize, size_t& valueSize) {
    Iterator* iter = findByKey(key, keySize);

    if (!iter) {
        valueSize = 0;
        return nullptr;
    }

    size_t elemSize;
    kv_pair* pair = (kv_pair*)iter->getElement(elemSize);

    if (!pair) {
        delete iter;
        valueSize = 0;
        return nullptr;
    }

    valueSize = pair->valueSize;
    void* result = pair->value;

    delete iter;
    return result;
}

Container::Iterator* Table::find(void* elem, size_t size) {
    if (!elem || size == 0) {
        return nullptr;
    }

    for (size_t i = 0; i < arraySize; i++) {
        if (!hashTable[i]) {
            continue;
        }

        Container::Iterator* iter = hashTable[i]->newIterator();

        if (!iter) {
            continue;
        }

        do {
            size_t elemSize;
            kv_pair* pair = (kv_pair*)iter->getElement(elemSize);

            if (pair && pair->valueSize == size && memcmp(pair->value, elem, size) == 0) {
                TableIterator* tableIter = new TableIterator(this, i);
                delete iter;
                return tableIter;
            }

            if (!iter->hasNext()) {
                break;
            }

            iter->goToNext();
        } while (true);

        delete iter;
    }

    return nullptr;
}

Table::TableIterator::TableIterator(Table* table, size_t startIndex)
    : GroupContainer::GroupContainerIterator(table, startIndex)
{
}

void* Table::TableIterator::getValue(size_t& size) {
    size_t elemSize;
    kv_pair* pair = (kv_pair*)getElement(elemSize);

    if (!pair) {
        size = 0;
        return nullptr;
    }

    size = pair->valueSize;
    return pair->value;
}

void* Table::TableIterator::getKey(size_t& size) {
    size_t elemSize;
    kv_pair* pair = (kv_pair*)getElement(elemSize);

    if (!pair) {
        size = 0;
        return nullptr;
    }

    size = pair->keySize;
    return pair->key;
}