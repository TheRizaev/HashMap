#include "Table.h"
#include <cstring> // для memcmp

// Конструктор таблицы
Table::Table(MemoryManager& mem)
    : AbstractTable(mem), GroupContainer(mem)
{
    // Все необходимые действия уже выполнены в конструкторах базовых классов
}

// Деструктор таблицы
Table::~Table()
{
    clear(); // Очищаем таблицу
    // Освобождение памяти для массива выполняется в деструкторе GroupContainer
}

// Преобразование ключа в строку для хеширования
std::string Table::keyToString(void* key, size_t keySize)
{
    if (!key || keySize == 0)
        return "";

    // Преобразуем ключ в строку
    char* keyPtr = static_cast<char*>(key);
    // Преобразуем в std::string (не включая нулевой символ в конце, если он есть)
    return std::string(keyPtr, keySize - 1);
}

// Перехеширование таблицы (увеличение размера)
bool Table::reHash()
{
    size_t oldSize = arraySize;
    size_t newSize = arraySize * 2;

    // Сохраняем старую таблицу
    List** oldTable = hashTable;

    // Создаем новую таблицу
    hashTable = (List**)_memory.allocMem(sizeof(List*) * newSize);
    if (!hashTable) {
        hashTable = oldTable;  // Восстанавливаем старую таблицу
        return false;
    }

    // Инициализируем новую таблицу
    for (size_t i = 0; i < newSize; i++) {
        hashTable[i] = nullptr;
    }

    // Обновляем размер массива
    arraySize = newSize;

    // Сбрасываем счетчик элементов (будем заново считать при перехешировании)
    amountOfElements = 0;

    // Перехешируем элементы из старой таблицы
    for (size_t i = 0; i < oldSize; i++) {
        if (oldTable[i]) {
            Container::Iterator* iter = oldTable[i]->newIterator();
            if (iter) {
                while (true) {
                    size_t elemSize;
                    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(elemSize));

                    if (pair) {
                        // Вычисляем новый хеш и вставляем элемент
                        std::string keyStr = keyToString(pair->key, pair->keySize);
                        size_t hash = hashFunc(keyStr) % arraySize;

                        // Если в данной позиции еще нет списка, создаем его
                        if (!hashTable[hash]) {
                            hashTable[hash] = new List(_memory);
                        }

                        // Вставляем пару в список
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

            // Удаляем старый список
            delete oldTable[i];
        }
    }

    // Освобождаем память, выделенную для старой таблицы
    _memory.freeMem(oldTable);

    return true;
}

// Вставка элемента по ключу
int Table::insertByKey(void* key, size_t keySize, void* value, size_t valueSize)
{
    // Проверяем, существует ли уже элемент с таким ключом
    size_t bucketIndex;
    if (findPairByKey(key, keySize, bucketIndex) != nullptr)
    {
        // Ключ уже существует, возвращаем ошибку
        return 1;
    }

    // Вычисляем хеш ключа
    std::string keyStr = keyToString(key, keySize);
    size_t hash = hashFunc(keyStr) % arraySize;

    // Если нагрузка превышает порог, увеличиваем размер таблицы
    if (getLoadFactor() > 0.75) {
        reHash();
        // Пересчитываем хеш после изменения размера таблицы
        hash = hashFunc(keyStr) % arraySize;
    }

    // Если в данной позиции еще нет списка, создаем его
    if (!hashTable[hash]) {
        hashTable[hash] = new List(_memory);
    }

    // Создаем пару ключ-значение
    KeyValuePair pair;

    // Выделяем память для ключа и значения
    pair.key = _memory.allocMem(keySize);
    if (!pair.key) {
        return 1; // Ошибка выделения памяти
    }

    pair.keySize = keySize;

    pair.value = _memory.allocMem(valueSize);
    if (!pair.value) {
        _memory.freeMem(pair.key);
        return 1;
    }

    pair.valueSize = valueSize;

    // Копируем данные ключа и значения
    memcpy(pair.key, key, keySize);
    memcpy(pair.value, value, valueSize);

    // Добавляем пару в список
    int result = hashTable[hash]->push_front(&pair, sizeof(KeyValuePair));

    // Проверяем результат операции
    if (result != 0) {
        // Если произошла ошибка, освобождаем выделенную память
        _memory.freeMem(pair.key);
        _memory.freeMem(pair.value);
    }
    else {
        // Увеличиваем счетчик элементов
        increaseAmount();
    }

    return result;
}

// Получение значения по ключу
void* Table::at(void* key, size_t keySize, size_t& valueSize)
{
    size_t bucketIndex;
    Container::Iterator* iter = findPairByKey(key, keySize, bucketIndex);
    if (!iter)
    {
        // Ключ не найден
        valueSize = 0;
        return nullptr;
    }

    // Получаем пару ключ-значение
    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

    // Сохраняем размер значения
    valueSize = pair->valueSize;

    // Сохраняем указатель на значение
    void* result = pair->value;

    // Удаляем итератор
    delete iter;

    // Возвращаем указатель на значение
    return result;
}

// Удаление элемента по ключу
void Table::removeByKey(void* key, size_t keySize)
{
    size_t bucketIndex;
    Container::Iterator* iter = findPairByKey(key, keySize, bucketIndex);
    if (!iter)
    {
        // Ключ не найден
        return;
    }

    // Получаем пару ключ-значение для освобождения памяти
    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

    // Освобождаем память для ключа и значения
    _memory.freeMem(pair->key);
    _memory.freeMem(pair->value);

    // Удаляем элемент из списка
    hashTable[bucketIndex]->remove(iter);

    // Уменьшаем счетчик элементов
    decreaseAmount();

    // Если список стал пустым, удаляем его
    if (hashTable[bucketIndex]->empty()) {
        delete hashTable[bucketIndex];
        hashTable[bucketIndex] = nullptr;
    }

    // Итератор уже удален в hashTable[bucketIndex]->remove(iter)
}

// Поиск пары ключ-значение по ключу
Container::Iterator* Table::findPairByKey(void* key, size_t keySize, size_t& bucketIndex)
{
    // Вычисляем хеш ключа
    std::string keyStr = keyToString(key, keySize);
    size_t hash = hashFunc(keyStr) % arraySize;
    bucketIndex = hash;

    // Проверяем, есть ли список в данной позиции
    if (!hashTable[hash])
    {
        // Список пуст
        return nullptr;
    }

    // Создаем итератор для списка
    Container::Iterator* iter = hashTable[hash]->newIterator();
    if (!iter)
    {
        // Не удалось создать итератор
        return nullptr;
    }

    // Обходим список и ищем пару с указанным ключом
    while (true)
    {
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

        if (!pair)
        {
            // Неожиданная ошибка
            delete iter;
            return nullptr;
        }

        // Сравниваем ключи
        if (pair->keySize == keySize && memcmp(pair->key, key, keySize) == 0)
        {
            // Ключ найден
            return iter;
        }

        // Переходим к следующему элементу
        if (iter->hasNext())
        {
            iter->goToNext();
        }
        else
        {
            // Достигнут конец списка, ключ не найден
            delete iter;
            return nullptr;
        }
    }
}

// Реализация метода findByKey из AbstractTable
Container::Iterator* Table::findByKey(void* key, size_t keySize)
{
    size_t bucketIndex;
    Container::Iterator* iter = findPairByKey(key, keySize, bucketIndex);
    if (!iter)
    {
        return nullptr;
    }

    // Создаем итератор таблицы, который начинается с найденного элемента
    TableIterator* tableIter = new TableIterator(this, bucketIndex);

    // Удаляем исходный итератор, так как мы создали новый
    delete iter;

    return tableIter;
}

// Поиск элемента по значению
Container::Iterator* Table::find(void* elem, size_t size)
{
    // Перебираем все списки и ищем значение
    for (size_t i = 0; i < arraySize; i++) {
        if (hashTable[i]) {
            Container::Iterator* iter = hashTable[i]->newIterator();
            if (iter) {
                while (true) {
                    size_t pairSize;
                    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

                    if (pair && pair->valueSize == size && memcmp(pair->value, elem, size) == 0) {
                        // Значение найдено
                        // Создаем итератор таблицы, который начинается с найденного элемента
                        TableIterator* tableIter = new TableIterator(this, i);

                        // Удаляем исходный итератор, так как мы создали новый
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

// Создание нового итератора
Container::Iterator* Table::newIterator()
{
    if (empty())
        return nullptr;

    return new TableIterator(this);
}

// Метод для удаления элемента (освобождение памяти ключа и значения)
void Table::removeElement(void* element, size_t elemSize)
{
    if (!element || elemSize != sizeof(KeyValuePair))
        return;

    KeyValuePair* pair = static_cast<KeyValuePair*>(element);
    _memory.freeMem(pair->key);
    _memory.freeMem(pair->value);
}

// Метод для очистки всех элементов в одной корзине
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
                // Освобождаем память для ключа и значения
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

// Реализация итератора таблицы

// Конструктор итератора таблицы
Table::TableIterator::TableIterator(GroupContainer* container, size_t startIndex)
    : GroupContainerIterator(container, startIndex)
{
}

// Переопределяем getElement, чтобы возвращать только значение
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

    // Возвращаем значение и его размер
    size = pair->valueSize;
    return pair->value;
}