#include "Table.h"
#include <cstring>

const double hashTable::MAX_LOAD_FACTOR = 0.75;

hashTable::hashTable(MemoryManager& mem) : AbstractTable(mem) {
    // Инициализация уже происходит в конструкторе GroupContainer
}

hashTable::~hashTable() {
    // Очистка выполняется в базовом классе
    clear();
}

// Преобразовать ключ в char* для хеширования
char* hashTable::keyToCharArray(void* key, size_t keySize) {
    char* keyChars = (char*)_memory.allocMem(keySize + 1);
    if (!keyChars) return nullptr;

    memcpy(keyChars, key, keySize);
    keyChars[keySize] = '\0'; // Добавление нулевого символа для безопасности

    return keyChars;
}

bool hashTable::reHash() {
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

    List** tempTable = Table;
    Table = newTable;
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

void hashTable::removeElement(void* element, size_t elemSize) {
    if (!element) return;

    kv_pair* pair = (kv_pair*)element;

    if (pair->key) {
        _memory.freeMem(pair->key);
    }

    if (pair->value) {
        _memory.freeMem(pair->value);
    }
}

void hashTable::clearBucket(size_t bucketIndex) {
    if (!Table || bucketIndex >= arraySize || !Table[bucketIndex]) {
        return;
    }

    List* bucket = Table[bucketIndex];
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

int hashTable::insertByKey(void* key, size_t keySize, void* elem, size_t elemSize) {
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

    if (!Table[hash]) {
        Table[hash] = new List(_memory);
    }

    kv_pair* pair = (kv_pair*)_memory.allocMem(sizeof(kv_pair));

    pair->key = _memory.allocMem(keySize);
    memcpy(pair->key, key, keySize);
    pair->keySize = keySize;

    pair->value = _memory.allocMem(elemSize);
    memcpy(pair->value, elem, elemSize);
    pair->valueSize = elemSize;

    int result = Table[hash]->push_front(pair, sizeof(kv_pair));

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

void hashTable::removeByKey(void* key, size_t keySize) {
    Iterator* iter = findByKey(key, keySize);

    if (iter != nullptr) {
        remove(iter);
        delete iter;
    }
}

Container::Iterator* hashTable::findByKey(void* key, size_t keySize) {
    if (!key || keySize == 0) {
        return nullptr;
    }

    char* keyChars = keyToCharArray(key, keySize);
    if (!keyChars) return nullptr;

    size_t hash = hashFunc(keyChars, keySize) % arraySize;
    _memory.freeMem(keyChars);

    if (!Table[hash]) {
        return nullptr;
    }

    Container::Iterator* iter = Table[hash]->newIterator();

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

void* hashTable::at(void* key, size_t keySize, size_t& valueSize) {
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

Container::Iterator* hashTable::find(void* elem, size_t size) {
    if (!elem || size == 0) {
        return nullptr;
    }

    for (size_t i = 0; i < arraySize; i++) {
        if (!Table[i]) {
            continue;
        }

        Container::Iterator* iter = Table[i]->newIterator();

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

hashTable::TableIterator::TableIterator(hashTable* table, size_t startIndex)
    : GroupContainer::GroupContainerIterator(table, startIndex)
{
}

void* hashTable::TableIterator::getValue(size_t& size) {
    size_t elemSize;
    kv_pair* pair = (kv_pair*)getElement(elemSize);

    if (!pair) {
        size = 0;
        return nullptr;
    }

    size = pair->valueSize;
    return pair->value;
}

void* hashTable::TableIterator::getKey(size_t& size) {
    size_t elemSize;
    kv_pair* pair = (kv_pair*)getElement(elemSize);

    if (!pair) {
        size = 0;
        return nullptr;
    }

    size = pair->keySize;
    return pair->key;
}