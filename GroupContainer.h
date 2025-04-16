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
    double getLoadFactor() {
        if (arraySize == 0) return 0.0;
        return static_cast<double>(amountOfElements) / arraySize;
    }

    // Изменение счетчика элементов
    void increaseAmount() { amountOfElements++; }
    void decreaseAmount() { if (amountOfElements > 0) amountOfElements--; }

    // Перехеширование - увеличение размера таблицы и перераспределение элементов
    virtual bool reHash() = 0;

    // Хеш-функция для ключа (строкового представления данных)
    size_t hashFunc(const std::string& key) {
        size_t hash = 0;
        for (char c : key) {
            hash = hash * 31 + c;
        }
        return hash;
    }

    // Доступ к внутренним данным
    size_t getArraySize() const { return arraySize; }
    List** getTable() const { return hashTable; }

public:
    // Конструктор
    GroupContainer(MemoryManager& mem) : Container(mem) {
        amountOfElements = 0;
        arraySize = 1000;
        hashTable = (List**)_memory.allocMem(sizeof(List*) * arraySize);
        for (size_t i = 0; i < arraySize; i++) {
            hashTable[i] = nullptr;
        }
    }

    // Деструктор
    virtual ~GroupContainer() {
        if (hashTable) {
            _memory.freeMem(hashTable);
            hashTable = nullptr;
        }
    }

    // Методы из базового класса Container
    bool empty() override { return amountOfElements == 0; }
    int size() override { return amountOfElements; }
    size_t max_bytes() override { return arraySize * sizeof(List*); }

    // Общий итератор для всех групповых контейнеров
    class GroupContainerIterator : public Iterator {
    protected:
        List* currentList;
        size_t index;
        Iterator* listIterator;
        GroupContainer* myContainer;

    public:
        // Конструктор
        GroupContainerIterator(GroupContainer* container, size_t startIndex = 0)
            : myContainer(container), index(startIndex), currentList(nullptr), listIterator(nullptr)
        {
            if (!myContainer || !myContainer->hashTable)
                return;

            // Находим первый непустой список
            for (size_t i = startIndex; i < myContainer->arraySize; i++) {
                if (myContainer->hashTable[i]) {
                    currentList = myContainer->hashTable[i];
                    listIterator = currentList->newIterator();
                    index = i;
                    break;
                }
            }
        }

        // Деструктор
        ~GroupContainerIterator() {
            if (listIterator) {
                delete listIterator;
                listIterator = nullptr;
            }
        }

        // Получение текущего элемента
        void* getElement(size_t& size) override {
            if (!currentList || !listIterator) {
                size = 0;
                return nullptr;
            }
            return listIterator->getElement(size);
        }

        // Проверка наличия следующего элемента
        bool hasNext() override {
            if (!myContainer || !myContainer->hashTable)
                return false;

            // Если текущий список есть и в нем есть следующий элемент
            if (currentList && listIterator && listIterator->hasNext())
                return true;

            // Ищем следующий непустой список
            for (size_t i = index + 1; i < myContainer->arraySize; i++) {
                if (myContainer->hashTable[i])
                    return true;
            }

            return false;
        }

        // Переход к следующему элементу
        void goToNext() override {
            if (!myContainer || !myContainer->hashTable)
                return;

            // Если в текущем списке есть следующий элемент
            if (currentList && listIterator && listIterator->hasNext()) {
                listIterator->goToNext();
                return;
            }

            // Освобождаем текущий итератор
            if (listIterator) {
                delete listIterator;
                listIterator = nullptr;
            }

            // Ищем следующий непустой список
            for (size_t i = index + 1; i < myContainer->arraySize; i++) {
                if (myContainer->hashTable[i]) {
                    currentList = myContainer->hashTable[i];
                    listIterator = currentList->newIterator();
                    index = i;
                    return;
                }
            }

            // Если не нашли следующий список
            currentList = nullptr;
        }

        // Сравнение итераторов
        bool equals(Iterator* right) override {
            if (!right)
                return false;

            GroupContainerIterator* rightIter = dynamic_cast<GroupContainerIterator*>(right);
            if (!rightIter)
                return false;

            // Сравниваем указатели на контейнер и индекс
            if (myContainer != rightIter->myContainer || index != rightIter->index)
                return false;

            // Если оба итератора указывают на конец (нет текущего списка)
            if (!currentList && !rightIter->currentList)
                return true;

            // Если один из итераторов указывает на конец
            if (!currentList || !rightIter->currentList)
                return false;

            // Сравниваем итераторы списка
            if (!listIterator || !rightIter->listIterator)
                return false;

            return listIterator->equals(rightIter->listIterator);
        }

        friend class GroupContainer;
    };

    // Создание нового итератора
    virtual Iterator* newIterator() override {
        if (empty())
            return nullptr;
        return new GroupContainerIterator(this);
    }

    // Удаление элемента по итератору
    virtual void remove(Iterator* iter) override {
        if (!iter) return;

        GroupContainerIterator* gcIter = dynamic_cast<GroupContainerIterator*>(iter);
        if (!gcIter || !gcIter->currentList || !gcIter->listIterator) return;

        // Удаляем элемент из списка
        size_t elemSize;
        void* element = gcIter->listIterator->getElement(elemSize);
        if (element) {
            // Удаляем данные элемента (специфично для каждого контейнера)
            removeElement(element, elemSize);

            // Удаляем элемент из списка
            gcIter->currentList->remove(gcIter->listIterator);

            // Уменьшаем счетчик элементов
            decreaseAmount();

            // Если список стал пустым, удаляем его
            if (gcIter->currentList->empty()) {
                delete gcIter->currentList;
                hashTable[gcIter->index] = nullptr;
                gcIter->currentList = nullptr;
            }

            // Обнуляем указатель на итератор списка
            gcIter->listIterator = nullptr;

            // Переходим к следующему элементу
            gcIter->goToNext();
        }
    }

    // Очистка всего контейнера
    void clear() override {
        for (size_t i = 0; i < arraySize; i++) {
            if (hashTable[i]) {
                clearBucket(i);
                delete hashTable[i];
                hashTable[i] = nullptr;
            }
        }
        amountOfElements = 0;
    }

protected:
    // Метод для освобождения памяти элемента (должен быть переопределен в потомках)
    virtual void removeElement(void* element, size_t elemSize) = 0;

    // Метод для очистки всех элементов в одной корзине
    virtual void clearBucket(size_t bucketIndex) = 0;
};