#pragma once
#include "Container.h"
#include "AbstractList.h"
#include "List.h"
#include "MemoryManager.h"

// GroupContainer - промежуточный класс, общий для множества и таблицы
// Реализует базовые операции, используя внутренний список List
class GroupContainer : public Container
{
protected:
    AbstractList* _list;  // Внутренний список для хранения элементов

public:
    // Конструктор - принимает ссылку на менеджер памяти
    GroupContainer(MemoryManager& mem) : Container(mem)
    {
        _list = new List(mem);
    }

    // Деструктор
    virtual ~GroupContainer()
    {
        if (_list)
            delete _list;
    }

    // Проверка на пустоту контейнера
    virtual bool empty() override
    {
        return _list->empty();
    }

    // Получение размера контейнера (количество элементов)
    virtual int size() override
    {
        return _list->size();
    }

    // Очистка контейнера
    virtual void clear() override
    {
        _list->clear();
    }

    // Создание нового итератора
    virtual Iterator* newIterator() override
    {
        return _list->newIterator();
    }
};