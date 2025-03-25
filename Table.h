#pragma once
#include "AbstractTable.h"
#include "List.h"
#include "GroupContainer.h"

// Класс: ассоциативная таблица на основе односвязного списка
class Table : public AbstractTable
{
private:
    // Структура для хранения пары ключ-значение
    struct KeyValuePair
    {
        void* key;
        size_t keySize;
        void* value;
        size_t valueSize;

        KeyValuePair(void* k, size_t kSize, void* v, size_t vSize)
            : key(k), keySize(kSize), value(v), valueSize(vSize) {}

        ~KeyValuePair()
        {
            // Высвобождение ресурсов выполняется во внешнем коде
        }
    };

    List _list;  // Односвязный список для хранения пар ключ-значение

public:
    // Класс итератора для таблицы
    class TableIterator : public Container::Iterator
    {
    private:
        Container::Iterator* _listIterator;  // Итератор списка

    public:
        TableIterator(Container::Iterator* listIter) : _listIterator(listIter) {}

        ~TableIterator()
        {
            delete _listIterator;
        }

        // Возвращает текущий элемент и его размер
        void* getElement(size_t& size) override;

        // Проверяет наличие следующего элемента
        bool hasNext() override;

        // Переходит к следующему элементу
        void goToNext() override;

        // Проверяет равенство итераторов
        bool equals(Container::Iterator* right) override;

        // Получить итератор списка
        Container::Iterator* getListIterator() { return _listIterator; }
    };

    // Конструктор
    Table(MemoryManager& mem);

    // Деструктор
    ~Table() override;

    // Реализация методов Container
    int size() override;
    size_t max_bytes() override;
    Container::Iterator* find(void* elem, size_t size) override;
    Container::Iterator* newIterator() override;
    void remove(Container::Iterator* iter) override;
    void clear() override;
    bool empty() override;

    // Реализация методов AbstractTable
    int insertByKey(void* key, size_t keySize, void* elem, size_t elemSize) override;
    void removeByKey(void* key, size_t keySize) override;
    Container::Iterator* findByKey(void* key, size_t keySize) override;
    void* at(void* key, size_t keySize, size_t& valueSize) override;

private:
    // Вспомогательная функция для сравнения ключей
    bool compareKeys(void* key1, size_t key1Size, void* key2, size_t key2Size);

    // Вспомогательная функция для создания пары ключ-значение
    KeyValuePair* createPair(void* key, size_t keySize, void* value, size_t valueSize);
};