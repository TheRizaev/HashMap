#pragma once
#include "Container.h"
#include <cstring>

// Базовый класс для некоторой группы абстрактных контейнеров (Множество и Ассоциативная таблица)
class GroupContainer : public Container
{
protected:
    // Общая вспомогательная функция для сравнения элементов
    bool compareElements(void* elem1, size_t size1, void* elem2, size_t size2) const
    {
        return size1 == size2 && memcmp(elem1, elem2, size1) == 0;
    }

    // Общая вспомогательная функция для копирования данных
    void* allocateAndCopy(void* src, size_t size)
    {
        if (src == nullptr || size == 0)
            return nullptr;

        void* dst = _memory.allocMem(size);
        if (dst == nullptr)
            return nullptr;

        memcpy(dst, src, size);
        return dst;
    }

public:
    GroupContainer(MemoryManager& mem) : Container(mem) {}
    virtual ~GroupContainer() {}
};