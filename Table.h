#pragma once
#include "AbstractTable.h"
#include "GroupContainer.h"
#include <string>

// Структура хранения пары ключ-значение
typedef struct KeyValuePair
{
    void* key;
    size_t keySize;
    void* value;
    size_t valueSize;
} kv_pair;

// Класс хеш-таблицы, наследующийся от абстрактной таблицы
class Table : public AbstractTable
{
private:
    // Максимальный коэффициент загрузки, при превышении которого происходит перехеширование
    static const double MAX_LOAD_FACTOR;

protected:
    // Конвертация ключа в строковое представление для хеширования
    std::string keyToString(void* key, size_t keySize);

    // Перехеширование таблицы при превышении коэффициента загрузки
    bool reHash() override;

    // Освобождение памяти элемента (пары ключ-значение)
    void removeElement(void* element, size_t elemSize) override;

    // Очистка элементов в корзине
    void clearBucket(size_t bucketIndex) override;

public:
    // Конструктор
    Table(MemoryManager& mem);

    // Деструктор
    ~Table();

    // Методы AbstractTable
    int insertByKey(void* key, size_t keySize, void* elem, size_t elemSize) override;
    void removeByKey(void* key, size_t keySize) override;
    Iterator* findByKey(void* key, size_t keySize) override;
    void* at(void* key, size_t keySize, size_t& valueSize) override;

    // Методы Container
    Iterator* find(void* elem, size_t size) override;

    // Класс итератора для таблицы
    class TableIterator : public GroupContainer::GroupContainerIterator {
    public:
        // Конструктор
        TableIterator(Table* table, size_t startIndex = 0);

        // Получение значения (без ключа)
        void* getValue(size_t& size);

        // Получение ключа
        void* getKey(size_t& size);
    };
};