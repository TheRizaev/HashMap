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


char* hashTable::keyToCharArray(void* key, size_t keySize) {
    char* ptr = (char*)_memory.allocMem(keySize + 1);
    strcpy(ptr, (char*)key);
    return ptr;
}

void hashTable::removeElement(void* element, size_t elemSize)
{
    kv_pair* pair = static_cast<kv_pair*>(element);
    if (pair) {
        if (pair->key) {
            _memory.freeMem(pair->key);
            pair->key = nullptr;
        }
        if (pair->value) {
            _memory.freeMem(pair->value);
            pair->value = nullptr;
        }
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

    size_t hash = hashFunc(keyChars, keySize);
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
    if (!key || keySize == 0 || empty())
        return;

    size_t index = hashFunc((char*)key, keySize);

    if (!Table[index])
        return;

    List* currentList = Table[index];
    Iterator* iter = currentList->newIterator();

    if (!iter)
        return;

    bool removed = false;

    while (true) {
        size_t elemSize;
        void* element = iter->getElement(elemSize);

        if (element) {
            kv_pair* pair = static_cast<kv_pair*>(element);

            if (pair->keySize == keySize && memcmp(pair->key, key, keySize) == 0) {
                currentList->remove(iter);
                decreaseAmount();  
                removed = true;
                break;
            }
        }

        if (!iter->hasNext())
            break;

        iter->goToNext();
    }

    delete iter;

    if (removed && currentList->size() == 0) {
        delete currentList;
        Table[index] = nullptr;
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
            TableIterator* tableIter = new TableIterator(this, hash, iter);
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
                TableIterator* tableIter = new TableIterator(this, i, iter);
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

hashTable::TableIterator::TableIterator(hashTable* table, size_t startIndex, Iterator* new_it)
    : GroupContainer::GroupContainerIterator(table, startIndex, new_it)
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