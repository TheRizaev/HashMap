#include "Table.h"
#include <sstream>
#include <cstring>

// Максимальный коэффициент загрузки, при превышении которого происходит перехеширование
const double Table::MAX_LOAD_FACTOR = 0.75;

// Конструктор
Table::Table(MemoryManager& mem) : AbstractTable(mem) {
    // Базовая инициализация выполняется в конструкторе GroupContainer
}

// Деструктор
Table::~Table() {
    // Очистка памяти происходит в деструкторе GroupContainer и clear()
    clear();
}

// Конвертация ключа в строковое представление для хеширования
std::string Table::keyToString(void* key, size_t keySize) {
    std::stringstream ss;
    unsigned char* keyBytes = static_cast<unsigned char*>(key);

    for (size_t i = 0; i < keySize; i++) {
        ss << keyBytes[i];
    }

    return ss.str();
}

// Перехеширование таблицы при превышении коэффициента загрузки
bool Table::reHash() {
    // Сохраняем старую таблицу
    size_t oldSize = getArraySize();
    List** oldTable = getTable();

    // Создаем новую таблицу вдвое большего размера
    size_t newSize = oldSize * 2;
    List** newTable = (List**)_memory.allocMem(sizeof(List*) * newSize);

    if (!newTable) {
        return false; // Не удалось выделить память
    }

    // Инициализируем новую таблицу
    for (size_t i = 0; i < newSize; i++) {
        newTable[i] = nullptr;
    }

    // Временно меняем указатель и размер
    List** tempTable = hashTable;
    hashTable = newTable;
    size_t tempSize = arraySize;
    arraySize = newSize;

    // Сбрасываем счетчик элементов
    int oldCount = amountOfElements;
    amountOfElements = 0;

    // Переносим элементы из старой таблицы в новую
    for (size_t i = 0; i < oldSize; i++) {
        if (oldTable[i]) {
            Container::Iterator* iter = oldTable[i]->newIterator();

            if (iter) {
                do {
                    size_t elemSize;
                    kv_pair* pair = (kv_pair*)iter->getElement(elemSize);

                    if (pair) {
                        // Добавляем пару в новую таблицу
                        insertByKey(pair->key, pair->keySize, pair->value, pair->valueSize);

                        // Освобождаем память пары в старой таблице
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

            // Удаляем старый список
            delete oldTable[i];
        }
    }

    // Освобождаем память старой таблицы
    _memory.freeMem(oldTable);

    // Проверяем, что количество элементов не изменилось
    if (amountOfElements != oldCount) {
        // Что-то пошло не так при перехешировании
        return false;
    }

    return true;
}

// Освобождение памяти элемента (пары ключ-значение)
void Table::removeElement(void* element, size_t elemSize) {
    if (!element) return;

    kv_pair* pair = (kv_pair*)element;

    // Освобождаем память ключа и значения
    if (pair->key) {
        _memory.freeMem(pair->key);
    }

    if (pair->value) {
        _memory.freeMem(pair->value);
    }
}

// Очистка элементов в корзине
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
                // Освобождаем память ключа и значения
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

// Вставка элемента по ключу
int Table::insertByKey(void* key, size_t keySize, void* elem, size_t elemSize) {
    // Проверяем, существует ли уже элемент с таким ключом
    if (findByKey(key, keySize) != nullptr) {
        return 1; // Элемент с таким ключом уже существует
    }

    // Проверяем необходимость перехеширования
    if (getLoadFactor() >= MAX_LOAD_FACTOR) {
        reHash();
    }

    // Вычисляем хеш для ключа
    std::string keyStr = keyToString(key, keySize);
    size_t hash = hashFunc(keyStr) % arraySize;

    // Если корзина еще не создана, создаем ее
    if (!hashTable[hash]) {
        hashTable[hash] = new List(_memory);
    }

    // Создаем новую пару ключ-значение
    kv_pair* pair = (kv_pair*)_memory.allocMem(sizeof(kv_pair));

    // Выделяем память и копируем ключ
    pair->key = _memory.allocMem(keySize);
    memcpy(pair->key, key, keySize);
    pair->keySize = keySize;

    // Выделяем память и копируем значение
    pair->value = _memory.allocMem(elemSize);
    memcpy(pair->value, elem, elemSize);
    pair->valueSize = elemSize;

    // Добавляем пару в список
    int result = hashTable[hash]->push_front(pair, sizeof(kv_pair));

    // Если добавление прошло успешно, увеличиваем счетчик элементов
    if (result == 0) {
        increaseAmount();
    }
    else {
        // В случае неудачи освобождаем память
        _memory.freeMem(pair->key);
        _memory.freeMem(pair->value);
        _memory.freeMem(pair);
    }

    return result;
}

// Удаление элемента по ключу
void Table::removeByKey(void* key, size_t keySize) {
    // Находим элемент по ключу
    Iterator* iter = findByKey(key, keySize);

    if (iter != nullptr) {
        // Удаляем элемент с помощью родительского метода
        remove(iter);
        delete iter;
    }
}

// Поиск элемента по ключу
Container::Iterator* Table::findByKey(void* key, size_t keySize) {
    if (!key || keySize == 0) {
        return nullptr;
    }

    // Вычисляем хеш для ключа
    std::string keyStr = keyToString(key, keySize);
    size_t hash = hashFunc(keyStr) % arraySize;

    // Если корзина пуста, возвращаем nullptr
    if (!hashTable[hash]) {
        return nullptr;
    }

    // Получаем итератор для списка
    Container::Iterator* iter = hashTable[hash]->newIterator();

    if (!iter) {
        return nullptr;
    }

    // Ищем элемент с заданным ключом
    do {
        size_t elemSize;
        kv_pair* pair = (kv_pair*)iter->getElement(elemSize);

        if (pair && pair->keySize == keySize && memcmp(pair->key, key, keySize) == 0) {
            // Нашли элемент, создаем итератор таблицы
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

// Получение значения по ключу
void* Table::at(void* key, size_t keySize, size_t& valueSize) {
    // Находим элемент по ключу
    Iterator* iter = findByKey(key, keySize);

    if (!iter) {
        valueSize = 0;
        return nullptr;
    }

    // Получаем пару ключ-значение
    size_t elemSize;
    kv_pair* pair = (kv_pair*)iter->getElement(elemSize);

    if (!pair) {
        delete iter;
        valueSize = 0;
        return nullptr;
    }

    // Устанавливаем размер значения
    valueSize = pair->valueSize;
    void* result = pair->value;

    delete iter;
    return result;
}

// Поиск элемента по значению
Container::Iterator* Table::find(void* elem, size_t size) {
    if (!elem || size == 0) {
        return nullptr;
    }

    // Проходим по всем корзинам
    for (size_t i = 0; i < arraySize; i++) {
        if (!hashTable[i]) {
            continue;
        }

        // Получаем итератор для списка
        Container::Iterator* iter = hashTable[i]->newIterator();

        if (!iter) {
            continue;
        }

        // Ищем элемент с заданным значением
        do {
            size_t elemSize;
            kv_pair* pair = (kv_pair*)iter->getElement(elemSize);

            if (pair && pair->valueSize == size && memcmp(pair->value, elem, size) == 0) {
                // Нашли элемент, создаем итератор таблицы
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

// Конструктор итератора таблицы
Table::TableIterator::TableIterator(Table* table, size_t startIndex)
    : GroupContainer::GroupContainerIterator(table, startIndex)
{
}

// Получение значения (без ключа)
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

// Получение ключа
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