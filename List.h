#pragma once
#include "AbstractList.h"

// Класс: однонаправленный список
class List : public AbstractList
{
private:
    // Структура узла списка
    struct Node
    {
        void* data;       // Указатель на данные
        size_t dataSize;  // Размер данных
        Node* next;       // Указатель на следующий узел

        Node(void* elem, size_t elemSize, Node* nextNode = nullptr) : next(nextNode), dataSize(elemSize)
        {
            data = elem;
        }

        ~Node()
        {
            // Высвобождение ресурсов выполняется во внешнем коде
        }
    };

    Node* _head;  // Указатель на начало списка
    int _size;    // Количество элементов в списке

public:
    // Класс итератора для списка
    class ListIterator : public Container::Iterator
    {
    private:
        Node* _current;  // Текущий узел
        Node* _prev;     // Предыдущий узел (для удаления)
        List* _list;     // Указатель на контейнер

    public:
        ListIterator(List* list, Node* current, Node* prev = nullptr)
            : _list(list), _current(current), _prev(prev) {}

        // Возвращает текущий элемент и его размер
        void* getElement(size_t& size) override;

        // Проверяет наличие следующего элемента
        bool hasNext() override;

        // Переходит к следующему элементу
        void goToNext() override;

        // Проверяет равенство итераторов
        bool equals(Container::Iterator* right) override;

        // Получить текущий узел
        Node* getCurrent() { return _current; }

        // Получить предыдущий узел
        Node* getPrev() { return _prev; }

        // Установить текущий узел (для безопасного обновления после удаления)
        void setCurrent(Node* node) { _current = node; }
    };

    // Конструктор
    List(MemoryManager& mem);

    // Деструктор
    ~List() override;

    // Реализация методов Container
    int size() override;
    size_t max_bytes() override;
    Container::Iterator* find(void* elem, size_t size) override;
    Container::Iterator* newIterator() override;
    void remove(Container::Iterator* iter) override;
    void clear() override;
    bool empty() override;

    // Реализация методов AbstractList
    int push_front(void* elem, size_t elemSize) override;
    void pop_front() override;
    void* front(size_t& size) override;
    int insert(Container::Iterator* iter, void* elem, size_t elemSize) override;

protected:
    // Вспомогательный метод для сравнения элементов
    bool compareElements(void* elem1, size_t size1, void* elem2, size_t size2) const
    {
        return size1 == size2 && memcmp(elem1, elem2, size1) == 0;
    }
};