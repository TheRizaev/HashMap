#include "GroupContainer.h"
#include <string>

// Конструктор
GroupContainer::GroupContainer(MemoryManager& mem) : Container(mem) {
    amountOfElements = 0;
    arraySize = 1000;
    hashTable = (List**)_memory.allocMem(sizeof(List*) * arraySize);
    for (size_t i = 0; i < arraySize; i++) {
        hashTable[i] = nullptr;
    }
}

// Деструктор
GroupContainer::~GroupContainer() {
    if (hashTable) {
        _memory.freeMem(hashTable);
        hashTable = nullptr;
    }
}

// Получение коэффициента загрузки хеш-таблицы
double GroupContainer::getLoadFactor() {
    if (arraySize == 0) return 0.0;
    return static_cast<double>(amountOfElements) / arraySize;
}

// Изменение счетчика элементов
void GroupContainer::increaseAmount() {
    amountOfElements++;
}

void GroupContainer::decreaseAmount() {
    if (amountOfElements > 0) amountOfElements--;
}

// Хеш-функция для ключа (строкового представления данных)
size_t GroupContainer::hashFunc(const std::string& key) {
    size_t hash = 0;
    for (char c : key) {
        hash = hash * 31 + c;
    }
    return hash;
}

// Доступ к внутренним данным
size_t GroupContainer::getArraySize() const {
    return arraySize;
}

List** GroupContainer::getTable() const {
    return hashTable;
}

// Методы из базового класса Container
bool GroupContainer::empty() {
    return amountOfElements == 0;
}

int GroupContainer::size() {
    return amountOfElements;
}

size_t GroupContainer::max_bytes() {
    return arraySize * sizeof(List*);
}

// Конструктор итератора
GroupContainer::GroupContainerIterator::GroupContainerIterator(GroupContainer* container, size_t startIndex)
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

// Деструктор итератора
GroupContainer::GroupContainerIterator::~GroupContainerIterator() {
    if (listIterator) {
        delete listIterator;
        listIterator = nullptr;
    }
}

// Получение текущего элемента
void* GroupContainer::GroupContainerIterator::getElement(size_t& size) {
    if (!currentList || !listIterator) {
        size = 0;
        return nullptr;
    }
    return listIterator->getElement(size);
}

// Проверка наличия следующего элемента
bool GroupContainer::GroupContainerIterator::hasNext() {
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
void GroupContainer::GroupContainerIterator::goToNext() {
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
bool GroupContainer::GroupContainerIterator::equals(Iterator* right) {
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

// Создание нового итератора
Iterator* GroupContainer::newIterator() {
    if (empty())
        return nullptr;
    return new GroupContainerIterator(this);
}

// Удаление элемента по итератору
void GroupContainer::remove(Iterator* iter) {
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
void GroupContainer::clear() {
    for (size_t i = 0; i < arraySize; i++) {
        if (hashTable[i]) {
            clearBucket(i);
            delete hashTable[i];
            hashTable[i] = nullptr;
        }
    }
    amountOfElements = 0;
}