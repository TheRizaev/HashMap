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
    // Проверяем, существует ли уже элемент с таким ключом
    Iterator* existingIter = findByKey(key, keySize);
    if (existingIter != nullptr) {
        delete existingIter;
        return 1; // Элемент с таким ключом уже существует
    }

    // Проверяем необходимость рехеширования
    if (getLoadFactor() >= MAX_LOAD_FACTOR) {
        reHash();
    }

    // Вычисляем хеш напрямую
    size_t hash = hashFunc((char*)key, keySize);

    // Создаем список если его нет
    if (!Table[hash]) {
        Table[hash] = new List(_memory);
    }

    // Создаем пару ключ-значение
    kv_pair* pair = (kv_pair*)_memory.allocMem(sizeof(kv_pair));

    pair->key = _memory.allocMem(keySize);
    memcpy(pair->key, key, keySize);
    pair->keySize = keySize;

    pair->value = _memory.allocMem(elemSize);
    memcpy(pair->value, elem, elemSize);
    pair->valueSize = elemSize;

    // Добавляем в список
    int result = Table[hash]->push_front(pair, sizeof(kv_pair));

    if (result == 0) {
        increaseAmount();
    }
    else {
        // Очищаем память в случае ошибки
        _memory.freeMem(pair->key);
        _memory.freeMem(pair->value);
        _memory.freeMem(pair);
    }

    return result;
}

void hashTable::removeByKey(void* key, size_t keySize) {
    if (!key || keySize == 0 || empty())
        return;

    // Вычисляем хеш напрямую из ключа
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
                // Очищаем память пары перед удалением
                removeElement(element, elemSize);

                // Удаляем из списка
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

    // Удаляем пустой список
    if (removed && currentList->size() == 0) {
        Table[index] = nullptr;
    }
}

Container::Iterator* hashTable::findByKey(void* key, size_t keySize) {
    if (!key || keySize == 0) {
        return nullptr;
    }

    // Вычисляем хеш напрямую
    size_t hash = hashFunc((char*)key, keySize);

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

void hashTable::remove(Iterator* iter) {
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
        gcIter->listIterator = gcIter->currentList->newIterator();
    }
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

bool hashTable::reHash() {
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
