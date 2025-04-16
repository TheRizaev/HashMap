#pragma once
#include "Container.h"
#include "List.h"
#include "MemoryManager.h"
#include <string>

// Базовый класс для групповых контейнеров (множество, таблица)
class GroupContainer : public Container
{
protected:
    int amountOfElements;
    size_t arraySize;
    List** hashTable;

    // Получение коэффициента загрузки хеш-таблицы
    double getLoadFactor();

    // Изменение счетчика элементов
    void increaseAmount();
    void decreaseAmount();

    // Перехеширование - увеличение размера таблицы и перераспределение элементов
    virtual bool reHash() = 0;

    // Хеш-функция для ключа (строкового представления данных)
    size_t hashFunc(const std::string& key);

    // Доступ к внутренним данным
    size_t getArraySize() const;
    List** getTable() const;

public:
    // Конструктор
    GroupContainer(MemoryManager& mem);

    // Деструктор
    virtual ~GroupContainer();

    // Методы из базового класса Container
    bool empty() override;
    int size() override;
    size_t max_bytes() override;

    // Общий итератор для всех групповых контейнеров
    class GroupContainerIterator : public Iterator {
    protected:
        List* currentList;
        size_t index;
        Iterator* listIterator;
        GroupContainer* myContainer;

    public:
        // Конструктор
        GroupContainerIterator(GroupContainer* container, size_t startIndex = 0);

        // Деструктор
        ~GroupContainerIterator();

        // Получение текущего элемента
        void* getElement(size_t& size) override;

        // Проверка наличия следующего элемента
        bool hasNext() override;

        // Переход к следующему элементу
        void goToNext() override;

        // Сравнение итераторов
        bool equals(Iterator* right) override;

        friend class GroupContainer;
    };

    // Создание нового итератора
    virtual Iterator* newIterator() override;

    // Удаление элемента по итератору
    virtual void remove(Iterator* iter) override;

    // Очистка всего контейнера
    void clear() override;

protected:
    // Метод для освобождения памяти элемента (должен быть переопределен в потомках)
    virtual void removeElement(void* element, size_t elemSize) = 0;

    // Метод для очистки всех элементов в одной корзине
    virtual void clearBucket(size_t bucketIndex) = 0;
};