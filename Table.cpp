#include "Table.h"
#include <cstring> // для memcmp

// Конструктор таблицы
Table::Table(MemoryManager& mem)
    : AbstractTable(mem)
{
    // Создаем внутренний список
    _list = new List(mem);
}

// Деструктор таблицы
Table::~Table()
{
    clear(); // Очищаем таблицу
    delete _list; // Удаляем список
}

// Методы базового класса Container
bool Table::empty()
{
    return _list->empty();
}

int Table::size()
{
    return _list->size();
}

size_t Table::max_bytes()
{
    // Возвращаем максимальное количество байт, которое может хранить таблица
    // Используем значение из списка, если оно доступно
    return _list->max_bytes();
}

void Table::clear()
{
    // Перед очисткой списка нужно освободить память для всех ключей и значений
    Container::Iterator* iter = _list->newIterator();
    if (iter)
    {
        while (true)
        {
            size_t pairSize;
            KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));
            
            if (pair)
            {
                // Освобождаем память для ключа и значения
                _memory.freeMem(pair->key);
                _memory.freeMem(pair->value);
            }
            
            if (!iter->hasNext())
                break;
                
            iter->goToNext();
        }
        delete iter;
    }
    
    // Очищаем список
    _list->clear();
}

// Создание нового итератора таблицы
Container::Iterator* Table::newIterator()
{
    Container::Iterator* listIter = _list->newIterator();
    if (!listIter)
    {
        return nullptr;
    }
    
    return new TableIterator(listIter);
}

// Вставка элемента по ключу
int Table::insertByKey(void* key, size_t keySize, void* value, size_t valueSize)
{
    // Проверяем, существует ли уже элемент с таким ключом
    if (findPairByKey(key, keySize) != nullptr)
    {
        // Ключ уже существует, возвращаем ошибку
        return 1;
    }
    
    // Создаем временную структуру для пары ключ-значение
    KeyValuePair pair;
    
    // Выделяем память для ключа и значения с помощью менеджера памяти
    pair.key = _memory.allocMem(keySize);
    if (!pair.key) {
        return 1; // Ошибка выделения памяти
    }
    
    pair.keySize = keySize;
    
    pair.value = _memory.allocMem(valueSize);
    if (!pair.value) {
        _memory.freeMem(pair.key);
        return 1;
    }
    
    pair.valueSize = valueSize;
    
    // Копируем данные ключа и значения
    memcpy(pair.key, key, keySize);
    memcpy(pair.value, value, valueSize);
    
    // Добавляем пару в список
    // List::push_front создаст копию структуры pair
    int result = _list->push_front(&pair, sizeof(KeyValuePair));
    
    // Проверяем результат операции
    if (result != 0) {
        // Если произошла ошибка, освобождаем выделенную память
        _memory.freeMem(pair.key);
        _memory.freeMem(pair.value);
    }
    
    return result;
}

// Получение значения по ключу
void* Table::at(void* key, size_t keySize, size_t& valueSize)
{
    Container::Iterator* iter = findPairByKey(key, keySize);
    if (!iter)
    {
        // Ключ не найден
        valueSize = 0;
        return nullptr;
    }
    
    // Получаем пару ключ-значение
    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));
    
    // Сохраняем размер значения
    valueSize = pair->valueSize;
    
    // Сохраняем указатель на значение
    void* result = pair->value;
    
    // Удаляем итератор
    delete iter;
    
    // Возвращаем указатель на значение
    return result;
}

// Удаление элемента по ключу
void Table::removeByKey(void* key, size_t keySize)
{
    Container::Iterator* iter = findPairByKey(key, keySize);
    if (iter)
    {
        // Получаем пару ключ-значение для освобождения памяти
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));
        
        // Освобождаем память для ключа и значения
        _memory.freeMem(pair->key);
        _memory.freeMem(pair->value);
        
        // Удаляем элемент из списка
        _list->remove(iter);
        
        // Удаляем итератор
        delete iter;
    }
}

// Поиск пары ключ-значение по ключу
Container::Iterator* Table::findPairByKey(void* key, size_t keySize)
{
    Container::Iterator* iter = _list->newIterator();
    if (!iter)
    {
        // Список пуст
        return nullptr;
    }
    
    // Обходим список и ищем пару с указанным ключом
    while (true)
    {
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));
        
        if (!pair)
        {
            // Неожиданная ошибка
            delete iter;
            return nullptr;
        }
        
        // Сравниваем ключи
        if (pair->keySize == keySize && memcmp(pair->key, key, keySize) == 0)
        {
            // Ключ найден
            return iter;
        }
        
        // Переходим к следующему элементу
        if (iter->hasNext())
        {
            iter->goToNext();
        }
        else
        {
            // Достигнут конец списка, ключ не найден
            delete iter;
            return nullptr;
        }
    }
}

// Реализация метода findByKey из AbstractTable
Container::Iterator* Table::findByKey(void* key, size_t keySize)
{
    Container::Iterator* iter = findPairByKey(key, keySize);
    if (!iter)
    {
        return nullptr;
    }
    
    // Создаем новый итератор таблицы
    TableIterator* tableIter = new TableIterator(iter);
    return tableIter;
}

// Поиск элемента по значению
Container::Iterator* Table::find(void* elem, size_t size)
{
    Container::Iterator* iter = _list->newIterator();
    if (!iter)
    {
        return nullptr;
    }
    
    // Обходим список и ищем пару с указанным значением
    while (true)
    {
        size_t pairSize;
        KeyValuePair* pair = static_cast<KeyValuePair*>(iter->getElement(pairSize));
        
        if (!pair)
        {
            delete iter;
            return nullptr;
        }
        
        // Сравниваем значения
        if (pair->valueSize == size && memcmp(pair->value, elem, size) == 0)
        {
            // Значение найдено
            return new TableIterator(iter);
        }
        
        // Переходим к следующему элементу
        if (iter->hasNext())
        {
            iter->goToNext();
        }
        else
        {
            // Достигнут конец списка, значение не найдено
            delete iter;
            return nullptr;
        }
    }
}

// Удаление элемента по итератору
void Table::remove(Iterator* iter)
{
    if (!iter) return;
    
    TableIterator* tableIter = dynamic_cast<TableIterator*>(iter);
    if (!tableIter || !tableIter->_listIterator) return;
    
    // Получаем пару ключ-значение для освобождения памяти
    size_t pairSize;
    KeyValuePair* pair = static_cast<KeyValuePair*>(tableIter->_listIterator->getElement(pairSize));
    
    if (pair)
    {
        // Освобождаем память для ключа и значения
        _memory.freeMem(pair->key);
        _memory.freeMem(pair->value);
        
        // Удаляем элемент из списка
        _list->remove(tableIter->_listIterator);
        
        // Обнуляем указатель на итератор списка, чтобы избежать двойного удаления
        tableIter->_listIterator = nullptr;
    }
}

// Реализация класса TableIterator

// Конструктор итератора таблицы
Table::TableIterator::TableIterator(Iterator* listIterator)
    : _listIterator(listIterator)
{
}

// Деструктор итератора таблицы
Table::TableIterator::~TableIterator()
{
    if (_listIterator)
    {
        delete _listIterator;
    }
}

// Получение текущего элемента (значения)
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
    
    // Возвращаем значение и его размер
    size = pair->valueSize;
    return pair->value;
}

// Проверка наличия следующего элемента
bool Table::TableIterator::hasNext()
{
    return _listIterator && _listIterator->hasNext();
}

// Переход к следующему элементу
void Table::TableIterator::goToNext()
{
    if (_listIterator)
    {
        _listIterator->goToNext();
    }
}

// Сравнение итераторов
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