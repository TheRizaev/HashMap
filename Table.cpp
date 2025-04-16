#include "Table.h"
#include <cstring> // ��� memcmp

// ����������� �������
Table::Table(MemoryManager& mem)
    : AbstractTable(mem)
{
    // ������� ���������� ������
    _list = new List(mem);
}

// ���������� �������
Table::~Table()
{
    clear(); // ������� �������
    delete _list; // ������� ������
}

// ������ �������� ������ Container
bool Table::empty()
{
    return _list->empty();
}

int Table::size()
{
    return _list->size();
}

void Table::clear()
{
    // ����� �������� ������ ����� ���������� ������ ��� ���� ������ � ��������
    Container::Iterator* iter = _list->newIterator();
    if (iter)
    {
        while (true)
        {
            size_t pairSize;
            KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));
            
            if (pair)
            {
                // ����������� ������ ��� ����� � ��������
                _memory.freeMem(pair->key);
                _memory.freeMem(pair->value);
            }
            
            if (!iter->hasNext())
                break;
                
            iter->goToNext();
        }
        delete iter;
    }
    
    // ������� ������
    _list->clear();
}

// �������� ������ ��������� �������
Container::Iterator* Table::newIterator()
{
    Container::Iterator* listIter = _list->newIterator();
    if (!listIter)
    {
        return nullptr;
    }
    
    return new TableIterator(listIter);
}

// ������� �������� �� �����
int Table::insertByKey(void* key, size_t keySize, void* value, size_t valueSize)
{
    // ���������, ���������� �� ��� ������� � ����� ������
    if (findPairByKey(key, keySize) != nullptr)
    {
        // ���� ��� ����������, ���������� ������
        return 1;
    }
    
    // ������� ��������� ��������� ��� ���� ����-��������
    KeyValuePair pair;
    
    // �������� ������ ��� ����� � �������� � ������� ��������� ������
    pair.key = _memory.allocMem(keySize);
    if (!pair.key) {
        return 1; // ������ ��������� ������
    }
    
    pair.keySize = keySize;
    
    pair.value = _memory.allocMem(valueSize);
    if (!pair.value) {
        _memory.freeMem(pair.key);
        return 1;
    }
    
    pair.valueSize = valueSize;
    
    // �������� ������ ����� � ��������
    memcpy(pair.key, key, keySize);
    memcpy(pair.value, value, valueSize);
    
    // ��������� ���� � ������
    // List::push_front ������� ����� ��������� pair
    int result = _list->push_front(&pair, sizeof(KeyValuePair));
    
    // ��������� ��������� ��������
    if (result != 0) {
        // ���� ��������� ������, ����������� ���������� ������
        _memory.freeMem(pair.key);
        _memory.freeMem(pair.value);
    }
    
    return result;
}

// ��������� �������� �� �����
void* Table::at(void* key, size_t keySize, size_t& valueSize)
{
    Container::Iterator* iter = findPairByKey(key, keySize);
    if (!iter)
    {
        // ���� �� ������
        valueSize = 0;
        return nullptr;
    }
    
    // �������� ���� ����-��������
    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));
    
    // ��������� ������ ��������
    valueSize = pair->valueSize;
    
    // ��������� ��������� �� ��������
    void* result = pair->value;
    
    // ������� ��������
    delete iter;
    
    // ���������� ��������� �� ��������
    return result;
}

// �������� �������� �� �����
void Table::removeByKey(void* key, size_t keySize)
{
    Container::Iterator* iter = findPairByKey(key, keySize);
    if (iter)
    {
        // �������� ���� ����-�������� ��� ������������ ������
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));
        
        // ����������� ������ ��� ����� � ��������
        _memory.freeMem(pair->key);
        _memory.freeMem(pair->value);
        
        // ������� ������� �� ������
        _list->remove(iter);
        
        // ������� ��������
        delete iter;
    }
}

// ����� ���� ����-�������� �� �����
Container::Iterator* Table::findPairByKey(void* key, size_t keySize)
{
    Container::Iterator* iter = _list->newIterator();
    if (!iter)
    {
        // ������ ����
        return nullptr;
    }
    
    // ������� ������ � ���� ���� � ��������� ������
    while (true)
    {
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));
        
        if (!pair)
        {
            // ����������� ������
            delete iter;
            return nullptr;
        }
        
        // ���������� �����
        if (pair->keySize == keySize && memcmp(pair->key, key, keySize) == 0)
        {
            // ���� ������
            return iter;
        }
        
        // ��������� � ���������� ��������
        if (iter->hasNext())
        {
            iter->goToNext();
        }
        else
        {
            // ��������� ����� ������, ���� �� ������
            delete iter;
            return nullptr;
        }
    }
}

// ���������� ������ TableIterator

// ����������� ��������� �������
Table::TableIterator::TableIterator(Iterator* listIterator)
    : _listIterator(listIterator)
{
}

// ���������� ��������� �������
Table::TableIterator::~TableIterator()
{
    if (_listIterator)
    {
        delete _listIterator;
    }
}

// ��������� �������� �������� (��������)
void* Table::TableIterator::getElement(size_t& size)
{
    if (!_listIterator)
    {
        size = 0;
        return nullptr;
    }
    
    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(_listIterator->getElement(pairSize));
    if (!pair)
    {
        size = 0;
        return nullptr;
    }
    
    // ���������� �������� � ��� ������
    size = pair->valueSize;
    return pair->value;
}

// �������� ������� ���������� ��������
bool Table::TableIterator::hasNext()
{
    return _listIterator && _listIterator->hasNext();
}

// ������� � ���������� ��������
void Table::TableIterator::goToNext()
{
    if (_listIterator)
    {
        _listIterator->goToNext();
    }
}

// ��������� ����������
bool Table::TableIterator::equals(Iterator* right)
{
    if (!right || !_listIterator)
    {
        return false;
    }
    
    TableIterator* rightIter = dynamic_cast<TableIterator*>(right);
    if (!rightIter)
    {
        return false;
    }
    
    return _listIterator->equals(rightIter->_listIterator);
}