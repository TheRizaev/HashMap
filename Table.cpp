#include "Table.h"
#include <cstring>

// ���������� ������� ��������� �������
void* Table::TableIterator::getElement(size_t& size)
{
    if (_listIterator == nullptr)
        return nullptr;

    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(_listIterator->getElement(pairSize));

    if (pair == nullptr)
        return nullptr;

    size = pair->valueSize;
    return pair->value;
}

bool Table::TableIterator::hasNext()
{
    return _listIterator != nullptr && _listIterator->hasNext();
}

void Table::TableIterator::goToNext()
{
    if (_listIterator != nullptr)
        _listIterator->goToNext();
}

bool Table::TableIterator::equals(Container::Iterator* right)
{
    TableIterator* rightIter = dynamic_cast<TableIterator*>(right);
    if (rightIter == nullptr)
        return false;

    return _listIterator->equals(rightIter->getListIterator());
}

// ���������� ������� �������
Table::Table(MemoryManager& mem) : AbstractTable(mem), _list(mem) {}

Table::~Table()
{
    clear();
}

int Table::size()
{
    return _list.size();
}

size_t Table::max_bytes()
{
    return _memory.maxBytes();
}

Container::Iterator* Table::find(void* elem, size_t size)
{
    Container::Iterator* listIter = _list.newIterator();
    if (listIter == nullptr)
        return nullptr;

    while (true)
    {
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(listIter->getElement(pairSize));

        if (pair == nullptr)
            break;

        if (compareElements(pair->value, pair->valueSize, elem, size))
            return new TableIterator(listIter);

        if (!listIter->hasNext())
            break;

        listIter->goToNext();
    }

    delete listIter;
    return nullptr;
}

Container::Iterator* Table::newIterator()
{
    Container::Iterator* listIter = _list.newIterator();
    if (listIter == nullptr)
        return nullptr;

    return new TableIterator(listIter);
}

void Table::remove(Container::Iterator* iter)
{
    TableIterator* tableIter = dynamic_cast<TableIterator*>(iter);
    if (tableIter == nullptr)
        return;

    Container::Iterator* listIter = tableIter->getListIterator();
    if (listIter == nullptr)
        return;

    // �������� ���� ����-�������� ����� ���������
    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(listIter->getElement(pairSize));

    if (pair == nullptr)
        return;

    // �������� ������, ������� ����� ����������
    void* keyToFree = pair->key;
    void* valueToFree = pair->value;
    void* pairToFree = pair;

    // ������� ������� �� ������
    _list.remove(listIter);

    // ����������� ������ ��� ����� � �������� ����� �������� �� ������
    _memory.freeMem(keyToFree);
    _memory.freeMem(valueToFree);
    _memory.freeMem(pairToFree);

    // ����������� �������� �������
    delete tableIter;
}

void Table::clear()
{
    Container::Iterator* iter = _list.newIterator();
    if (iter == nullptr)
        return;

    while (true)
    {
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));

        if (pair == nullptr)
            break;

        // ����������� ������ ��� ����� � ��������
        _memory.freeMem(pair->key);
        _memory.freeMem(pair->value);
        _memory.freeMem(pair);

        if (!iter->hasNext())
            break;

        iter->goToNext();
    }

    delete iter;
    _list.clear();
}

bool Table::empty()
{
    return _list.empty();
}

int Table::insertByKey(void* key, size_t keySize, void* elem, size_t elemSize)
{
    // ���������, ���������� �� ��� ������� � ����� ������
    Container::Iterator* iter = findByKey(key, keySize);
    if (iter != nullptr)
    {
        delete iter;
        return 1;  // ���� ��� ����������
    }

    try
    {
        // ������� ����� ����� � �������� ��������� ����� �� GroupContainer
        void* keyCopy = allocateAndCopy(key, keySize);
        if (keyCopy == nullptr)
            return 1;

        void* valueCopy = allocateAndCopy(elem, elemSize);
        if (valueCopy == nullptr)
        {
            _memory.freeMem(keyCopy);
            return 1;
        }

        // ������� ���� ����-��������
        KeyValuePair* pair = createPair(keyCopy, keySize, valueCopy, elemSize);
        if (pair == nullptr)
        {
            _memory.freeMem(keyCopy);
            _memory.freeMem(valueCopy);
            return 1;
        }

        // ��������� ���� � ������
        if (_list.push_front(pair, sizeof(KeyValuePair)) != 0)
        {
            _memory.freeMem(keyCopy);
            _memory.freeMem(valueCopy);
            _memory.freeMem(pair);
            return 1;
        }

        return 0;
    }
    catch (...)
    {
        return 1;
    }
}

void Table::removeByKey(void* key, size_t keySize)
{
    Container::Iterator* iter = findByKey(key, keySize);
    if (iter == nullptr)
        return;

    // �������� ������ � ���� ����-�������� �� ��������
    TableIterator* tableIter = dynamic_cast<TableIterator*>(iter);
    if (tableIter == nullptr) {
        delete iter;
        return;
    }

    // ������� �������
    remove(iter);

    // �� ������� �������� �����, ��� ��� remove() ��� ��������� �������
}

Container::Iterator* Table::findByKey(void* key, size_t keySize)
{
    Container::Iterator* listIter = _list.newIterator();
    if (listIter == nullptr)
        return nullptr;

    while (true)
    {
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(listIter->getElement(pairSize));

        if (pair == nullptr)
            break;

        if (compareKeys(pair->key, pair->keySize, key, keySize))
            return new TableIterator(listIter);

        if (!listIter->hasNext())
            break;

        listIter->goToNext();
    }

    delete listIter;
    return nullptr;
}

void* Table::at(void* key, size_t keySize, size_t& valueSize)
{
    Container::Iterator* iter = findByKey(key, keySize);
    if (iter == nullptr)
        return nullptr;

    void* result = iter->getElement(valueSize);
    delete iter;

    return result;
}

bool Table::compareKeys(void* key1, size_t key1Size, void* key2, size_t key2Size)
{
    return compareElements(key1, key1Size, key2, key2Size);
}

Table::KeyValuePair* Table::createPair(void* key, size_t keySize, void* value, size_t valueSize)
{
    KeyValuePair* pair = static_cast<KeyValuePair*>(_memory.allocMem(sizeof(KeyValuePair)));
    if (pair == nullptr)
        return nullptr;

    // ���������� placement new ��� ������ ������������
    new (pair) KeyValuePair(key, keySize, value, valueSize);

    return pair;
}