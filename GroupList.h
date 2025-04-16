#pragma once
#include "Container.h"

// Ѕазовый класс дл€ однонаправленного списка
class GroupList : public Container
{
public:
    GroupList(MemoryManager& mem) : Container(mem) {}
};