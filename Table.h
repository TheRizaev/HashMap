#pragma once
#include "AbstractTable.h"
#include "GroupContainer.h"
#include "List.h"
#include "MemoryManager.h"
#include <string>

// Структура для хранения пары ключ-значение
typedef struct KeyValuePair
{
    void* key;       // Указатель на ключ
    size_t keySize;  // Размер ключа в байтах
    void* value;     // Указатель на значение
    size_t valueSize; // Размер значения в байтах
} KeyValuePair;

// Класс ассоциативной таблицы, наследуемый от AbstractTable и GroupContainer
class Table : public AbstractTable, public GroupContainer
{
public:
    // Конструктор, принимающий менеджер памяти
    Table(MemoryManager& mem);

    // Деструктор
    ~Table();

    // Методы класса AbstractTable
    int insertByKey(void* key, size_t keySize, void* value, size_t valueSize) override;
    void* at(void* key, size_t keySize, size_t& valueSize) override;
    void removeByKey(void* key, size_t keySize) override;
    Iterator* findByKey(void* key, size_t keySize) override;

    // Метод из Container, переопределенный для поиска по значению
    Iterator* find(void* elem, size_t size) override;

    // Собственный итератор таблицы, наследуемый от GroupContainerIterator
    class TableIterator : public GroupContainer::GroupContainerIterator
    {
    public:
        // Конструктор
        TableIterator(GroupContainer* container, size_t startIndex = 0);

        // Переопределяем getElement, чтобы возвращать только значение
        void* getElement(size_t& size) override;

        friend class Table;
    };

    // Создание нового итератора таблицы
    Iterator* newIterator() override;

protected:
    // Метод для удаления элемента (освобождение памяти ключа и значения)
    void removeElement(void* element, size_t elemSize) override;

    // Метод для очистки всех элементов в одной корзине
    void clearBucket(size_t bucketIndex) override;

    // Перехеширование таблицы
    bool reHash() override;

private:
    // Преобразование ключа в строку для хеширования
    std::string keyToString(void* key, size_t keySize);

    // Поиск пары ключ-значение по ключу
    Container::Iterator* findPairByKey(void* key, size_t keySize, size_t& bucketIndex);
};