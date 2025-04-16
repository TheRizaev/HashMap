#pragma once
#include "AbstractTable.h"
#include "List.h"
#include "MemoryManager.h"

// Структура для хранения пары ключ-значение
typedef struct KeyValuePair
{
    void* key;       // Указатель на ключ
    size_t keySize;  // Размер ключа в байтах
    void* value;     // Указатель на значение
    size_t valueSize; // Размер значения в байтах
} KeyValuePair;

// Класс ассоциативной таблицы, наследуемый от AbstractTable
class Table : public AbstractTable
{
private:
    List* _list;     // Внутренний список для хранения элементов

public:
    // Конструктор, принимающий менеджер памяти
    Table(MemoryManager& mem);

    // Деструктор
    ~Table();

    // Методы базового класса Container
    bool empty() override;
    int size() override;
    void clear() override;
    Iterator* newIterator() override;

    // Методы класса AbstractTable
    int insertByKey(void* key, size_t keySize, void* value, size_t valueSize) override;
    void* at(void* key, size_t keySize, size_t& valueSize) override;
    void removeByKey(void* key, size_t keySize) override;

    // Класс-итератор для обхода ассоциативной таблицы
    class TableIterator : public Iterator
    {
    private:
        Iterator* _listIterator; // Внутренний итератор для обхода списка

    public:
        // Конструктор, принимающий итератор списка
        TableIterator(Iterator* listIterator);

        // Деструктор
        ~TableIterator();

        // Методы базового класса Iterator
        void* getElement(size_t& size) override;
        bool hasNext() override;
        void goToNext() override;
        bool equals(Iterator* right) override;
    };

private:
    // Поиск пары ключ-значение по ключу
    // Возвращает итератор на найденный элемент или nullptr, если ключ не найден
    Container::Iterator* findPairByKey(void* key, size_t keySize);
};