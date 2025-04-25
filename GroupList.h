#pragma once
#include "Container.h"

class GroupList : public Container
{
public:
    GroupList(MemoryManager& mem) : Container(mem) {}
};