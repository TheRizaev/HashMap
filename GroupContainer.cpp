#include <iostream>
#include <cstring>
#include <string>
#include "Table.h"
#include "Mem.h"
#include <locale.h>

// Класс для тестирования контейнеров
class ContainerTester
{
public:
    // Тестирование хеш-таблицы с целыми числами
    static void testIntTable()
    {
        std::cout << "\n=== Тестирование таблицы с целыми числами ===\n" << std::endl;

        // Создаем менеджер памяти
        Mem memory(10000);

        // Создаем таблицу
        Table table(memory);

        // Тестируем пустую таблицу
        std::cout << "Пустая таблица: " << (table.empty() ? "да" : "нет") << std::endl;
        std::cout << "Размер таблицы: " << table.size() << std::endl;

        // Добавляем элементы
        int keys[] = { 1, 2, 3, 4, 5 };
        int values[] = { 10, 20, 30, 40, 50 };

        for (int i = 0; i < 5; i++)
        {
            int result = table.insertByKey(&keys[i], sizeof(int), &values[i], sizeof(int));
            std::cout << "Добавление ключа " << keys[i] << ", значения " << values[i]
                << ": " << (result == 0 ? "успешно" : "ошибка") << std::endl;
        }

        // Проверяем размер
        std::cout << "Размер таблицы после добавления: " << table.size() << std::endl;
        std::cout << "Таблица пуста? " << (table.empty() ? "да" : "нет") << std::endl;

        // Получаем значения по ключу
        for (int i = 0; i < 5; i++)
        {
            size_t valueSize;
            int* value = (int*)table.at(&keys[i], sizeof(int), valueSize);

            if (value != nullptr)
            {
                std::cout << "Значение для ключа " << keys[i] << ": " << *value << std::endl;
            }
            else
            {
                std::cout << "Значение для ключа " << keys[i] << " не найдено" << std::endl;
            }
        }

        // Тестируем поиск несуществующего ключа
        int nonExistentKey = 999;
        size_t valueSize;
        int* value = (int*)table.at(&nonExistentKey, sizeof(int), valueSize);
        std::cout << "Значение для несуществующего ключа " << nonExistentKey << ": "
            << (value != nullptr ? std::to_string(*value) : "не найдено") << std::endl;

        // Тестируем удаление
        int keyToRemove = 3;
        table.removeByKey(&keyToRemove, sizeof(int));
        std::cout << "Удален ключ " << keyToRemove << std::endl;
        std::cout << "Размер таблицы после удаления: " << table.size() << std::endl;

        // Проверяем, что элемент удален
        value = (int*)table.at(&keyToRemove, sizeof(int), valueSize);
        std::cout << "Значение для удаленного ключа " << keyToRemove << ": "
            << (value != nullptr ? std::to_string(*value) : "не найдено") << std::endl;

        // Тестируем итератор
        std::cout << "\nПеречисление всех элементов через итератор:" << std::endl;
        Container::Iterator* iter = table.newIterator();

        if (iter != nullptr)
        {
            do
            {
                size_t elemSize;
                void* elemValue = iter->getElement(elemSize);

                if (elemValue != nullptr)
                {
                    kv_pair* pair = (kv_pair*)elemValue;
                    int* k = (int*)pair->key;
                    int* v = (int*)pair->value;
                    std::cout << "Ключ: " << *k << ", Значение: " << *v << std::endl;
                }

                if (!iter->hasNext())
                {
                    break;
                }

                iter->goToNext();
            } while (true);

            delete iter;
        }

        // Очищаем таблицу
        table.clear();
        std::cout << "\nПосле очистки:" << std::endl;
        std::cout << "Размер таблицы: " << table.size() << std::endl;
        std::cout << "Таблица пуста? " << (table.empty() ? "да" : "нет") << std::endl;
    }

    // Тестирование хеш-таблицы со строками
    static void testStringTable()
    {
        std::cout << "\n=== Тестирование таблицы со строками ===\n" << std::endl;

        // Создаем менеджер памяти
        Mem memory(10000);

        // Создаем таблицу
        Table table(memory);

        // Добавляем элементы
        const char* keys[] = { "apple", "banana", "cherry", "date", "elderberry" };
        const char* values[] = { "red", "yellow", "red", "brown", "purple" };

        for (int i = 0; i < 5; i++)
        {
            int result = table.insertByKey((void*)keys[i], strlen(keys[i]) + 1,
                (void*)values[i], strlen(values[i]) + 1);
            std::cout << "Добавление ключа '" << keys[i] << "', значения '" << values[i]
                << "': " << (result == 0 ? "успешно" : "ошибка") << std::endl;
        }

        // Пробуем добавить дублирующийся ключ
        const char* duplicateKey = "apple";
        const char* newValue = "green";
        int result = table.insertByKey((void*)duplicateKey, strlen(duplicateKey) + 1,
            (void*)newValue, strlen(newValue) + 1);
        std::cout << "Добавление дублирующегося ключа '" << duplicateKey << "': "
            << (result == 0 ? "успешно" : "ошибка") << std::endl;

        // Получаем значения по ключу
        for (int i = 0; i < 5; i++)
        {
            size_t valueSize;
            char* value = (char*)table.at((void*)keys[i], strlen(keys[i]) + 1, valueSize);

            if (value != nullptr)
            {
                std::cout << "Значение для ключа '" << keys[i] << "': '" << value << "'" << std::endl;
            }
            else
            {
                std::cout << "Значение для ключа '" << keys[i] << "' не найдено" << std::endl;
            }
        }

        // Тестируем удаление
        const char* keyToRemove = "banana";
        table.removeByKey((void*)keyToRemove, strlen(keyToRemove) + 1);
        std::cout << "Удален ключ '" << keyToRemove << "'" << std::endl;

        // Проверяем, что элемент удален
        size_t valueSize;
        char* value = (char*)table.at((void*)keyToRemove, strlen(keyToRemove) + 1, valueSize);
        std::cout << "Значение для удаленного ключа '" << keyToRemove << "': "
            << (value != nullptr ? value : "не найдено") << std::endl;

        // Тестируем поиск по значению
        const char* valueToFind = "red";
        Container::Iterator* iter = table.find((void*)valueToFind, strlen(valueToFind) + 1);
        std::cout << "Поиск значения '" << valueToFind << "': "
            << (iter != nullptr ? "найдено" : "не найдено") << std::endl;

        if (iter != nullptr)
        {
            delete iter;
        }

        // Очищаем таблицу
        table.clear();
        std::cout << "После очистки размер таблицы: " << table.size() << std::endl;
    }

    // Тестирование перехеширования при достижении предельной загрузки
    static void testRehashing()
    {
        std::cout << "\n=== Тестирование перехеширования ===\n" << std::endl;

        // Создаем менеджер памяти
        Mem memory(100000);

        // Создаем таблицу
        Table table(memory);

        const int numElements = 2000; // Достаточно для срабатывания перехеширования

        std::cout << "Добавление " << numElements << " элементов..." << std::endl;

        // Добавляем элементы
        for (int i = 0; i < numElements; i++)
        {
            int key = i;
            int value = i * 10;
            table.insertByKey(&key, sizeof(int), &value, sizeof(int));

            // Выводим информацию о размере через некоторые интервалы
            if (i % 500 == 0 || i == numElements - 1)
            {
                std::cout << "Добавлено " << i + 1 << " элементов, размер таблицы: "
                    << table.size() << std::endl;
            }
        }

        // Проверяем доступ к случайным элементам
        const int numChecks = 20;
        std::cout << "\nПроверяем " << numChecks << " случайных элементов после перехеширования:" << std::endl;

        for (int i = 0; i < numChecks; i++)
        {
            int key = rand() % numElements;
            size_t valueSize;
            int* value = (int*)table.at(&key, sizeof(int), valueSize);

            std::cout << "Ключ " << key << ": "
                << (value != nullptr ? std::to_string(*value) : "не найдено") << std::endl;
        }

        // Очищаем таблицу
        table.clear();
    }

    // Тестирование сложных структур
    static void testStructs()
    {
        std::cout << "\n=== Тестирование со сложными структурами ===\n" << std::endl;

        // Определяем структуру
        struct Person
        {
            int id;
            char name[50];
            int age;
        };

        // Создаем менеджер памяти
        Mem memory(10000);

        // Создаем таблицу
        Table table(memory);

        // Создаем несколько объектов Person
        Person persons[] = {
            {1, "Alice", 25},
            {2, "Bob", 30},
            {3, "Charlie", 35},
            {4, "David", 40},
            {5, "Eve", 45}
        };

        // Добавляем элементы в таблицу
        for (int i = 0; i < 5; i++)
        {
            int result = table.insertByKey(&persons[i].id, sizeof(int), &persons[i], sizeof(Person));
            std::cout << "Добавление Person с id=" << persons[i].id << ": "
                << (result == 0 ? "успешно" : "ошибка") << std::endl;
        }

        // Получаем значения по ключу
        for (int i = 1; i <= 5; i++)
        {
            size_t valueSize;
            Person* person = (Person*)table.at(&i, sizeof(int), valueSize);

            if (person != nullptr)
            {
                std::cout << "Person с id=" << i << ": name='" << person->name
                    << "', age=" << person->age << std::endl;
            }
            else
            {
                std::cout << "Person с id=" << i << " не найден" << std::endl;
            }
        }

        // Удаляем один элемент
        int idToRemove = 3;
        table.removeByKey(&idToRemove, sizeof(int));
        std::cout << "Удален Person с id=" << idToRemove << std::endl;

        // Проверяем, что элемент удален
        size_t valueSize;
        Person* person = (Person*)table.at(&idToRemove, sizeof(int), valueSize);
        std::cout << "Person с id=" << idToRemove << (person != nullptr ? " найден" : " не найден") << std::endl;

        // Очищаем таблицу
        table.clear();
        std::cout << "После очистки размер таблицы: " << table.size() << std::endl;
    }
};

int main()
{
    setlocale(LC_ALL, "Russian");

    std::cout << "===== ТЕСТИРОВАНИЕ ХЕШ-ТАБЛИЦЫ =====\n" << std::endl;

    // Тестируем хеш-таблицу с разными типами данных
    ContainerTester::testIntTable();
    ContainerTester::testStringTable();
    ContainerTester::testRehashing();
    ContainerTester::testStructs();

    std::cout << "\n===== ТЕСТИРОВАНИЕ ЗАВЕРШЕНО =====\n" << std::endl;

    return 0;
}